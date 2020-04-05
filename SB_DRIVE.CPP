/*
This is an alternate form of SB_DRIVE.C.  Changes include:

	allow DMA buffers up to 64K
	keystroke stops DMA output
	interrupt changed
	init_timers initializes faster

Calling interface is through the following calls:

	dsp_open (&dev);
	dsp_set_speed (&speed);
	dsp_write (buf,len);
	dsp_close ();

Only non-pro SoundBlaster dsp writes implemented.

Bill Neisius (bill@solaria.hac.com) - July 1992

(original notes follow:)
*/

/*======================================================================

   Device driver for the Creative Labs Sound Blaster card.

   [ This file is part of the SBMSDOS v1.0 distibution ]

   Michael Fulbright (msf@as.arizona.edu)

   This file was originally distributed by the fellow below, I'm
   just borrowing it.

	=====================================================================
   ORIGINAL HEADER FOLLOWS (msf)
   =====================================================================

   [ This file is a part of SBlast-BSD-1.4 ]

   Steve Haehnichen <shaehnic@ucsd.edu>

   $Id: sb_driver.c,v 1.29 1992/06/13 01:46:43 steve Exp steve $

   Copyright (C) 1992 Steve Haehnichen.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 * $Log: sb_driver.c,v $
 * Revision 1.29  1992/06/13  01:46:43  steve
 * Released in SBlast-BSD-1.4

======================================================================*/

/* #pragma optimize("",off) */ /* compile without optimize, to preserve wait loops */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <mem.h>

#include "sblast.h"	     /* User-level structures and defs */
#include "sb_regs.h"	    /* Register and command values */
#include "dehacked.h"			  /* DeHackEd stuff (mouse) */

#include <malloc.h>

#if defined (FULL_DEBUG) && !defined (DEBUG)
#  define DEBUG
#endif

#ifdef DEBUG
#  define DPRINTF(x)    printf x;
#else
#  define DPRINTF(x)
#endif

#define min(a,b)    (((a) < (b)) ? (a) : (b))

int DMA_complete = 1;
/*
 * This is the highest DSP speed that will use "Low-Speed" mode.
 * Anything greater than this will use the "High-Speed" mode instead.
 */
#define	 MAX_LOW_SPEED   22222

/*
 * This is the speed the DSP will start at when you reboot the system.
 * If I can read the speed registers, then maybe I should just use
 * the current card setting.  Does it matter?
 * (43478 is the closest we can get to 44 KHz)
 */
#define	 INITIAL_DSP_SPEED       8000

/*
 * Define this if you consider ADC overruns to be an error, and
 * want to abort the read() call.  If you are making real recordings,
 * this is usually the case, or you will have gaps in the sound.
 * (This shouldn't happen.  Most apps can keep up with 45K/sec.)
 * For real-time use, overruns should be ignored.
 * This looks like an ioctl-toggle candidate to me.
 */
#define	 ERROR_ON_OVERRUN

#define FALSE 0
#define TRUE 1

#define GOOD 1
#define FAIL 0
#define ON   1
#define OFF  0


/* Number of "cycles" in one second.  Shouldn't this be somewhere else? */
#define HZ	      100

#define TIMEOUT	 (10 * HZ)  /* FM interrupt patience in clock ticks */

#define DSP_LOOP_MAX 1000

#define DSP_UNIT 0

enum { PLAY, RECORD };	  /* DSP sampling directions */

/* WVN - 64K buffers */
/*
 * NOTE - MSF - Well, I cant afford OS/2 yet, so I'm stuck with
 *	      wonderful 64k segments in MSC 5.1. Comment below
 *	      holds, except I allocate 2 buffers in a 32k chunk.
 *	      To be sure things will work DO NOT CHOOSE DSP_BUF_SIZE
 *	      to be larger than 1/3 sizeof(memory_chunk)!.
 *
 * ORIGINAL COMMENT FOLLOWS (msf)
 *
 * This is the DSP memory buffer size.  Choose wisely. :-)
 * I started with 21K buffers, and then later changed to 64K buffers.
 * This hogs more kernel memory, but gives fewer Pops and
 * needs servicing less often.  Adjust to taste.
 * Note that it must be an EVEN number if you want stereo to work.
 * (See dsp_find_buffers() for a better explanation
 * Note that the same amount of memory (192K) is hogged, regardless of
 * the buffer size you choose.  This makes it easier to allocate page-safe
 * buffers, but should probably be changed to attempt to pack more into
 * one 64K page.  Be really sure you understand what you are doing if
 * you change the size of memory_chunk[].
 * 64K is the biggest DSP_BUF_SIZE you can have.
 * Smaller buffer sizes make aborts and small reads more responsive.
 * For real-time stuff, perhaps ~10K would be better.
 */

/* buffer size can be any size up to 64K */
/* sizes between 32K and 64K are somewhat wastful of memory...*/
/* ... a buffer of 33K allocates 130K of memory */
/* ... a buffer of 32K allocates  96K of memory */
/* either select 64K, or a buffer 32K or less...*/
#define DSP_BUF_SIZE	(long)((1L << 16) -1)

static char _huge *memory_chunk;

/*
   for all DSP-related variables and flags. */
/* MSF - added speaker_on flag to tell if speaker on or off */

struct sb_dsp_type
{
  unsigned int  speed;	  /* DSP sampling rate */
  int   no_timeout;		/* Timeout for one DSP buffer */
  BYTE  compression;	    /* Current DAC decompression mode */
  FLAG  hispeed;		/* 1 = High Speed DSP mode, 0 = low speed */
  FLAG  in_stereo;	      /* 1 = currently in stereo, 0 = mono */
  BYTE  start_command;	  /* current DSP start command */
  int   error;		  /* Current error status on read/write */
  int   semaphore;	      /* Silly place-holder int for dsp_dma_start */
  void  (*cont_xfer)(void);     /* Function to call to continue a DMA xfer */
  char  _huge *buf[2];		/* Two pointers to mono-page buffers */
  unsigned long  phys_buf[2];	    /* Physical addresses for dsp buffers */
  FLAG  full[2];		/* True when that buffer is full */
  unsigned int   used[2];		/* Buffer bytes used by read/write */
  BYTE  active;		 /* The buffer currently engaged in DMA */
  BYTE  hi;		     /* The buffer being filled/emptied by user */
  FLAG  first_block;	    /* True for the first DAC block */
  FLAG  dont_block;	     /* FNDELAY flag sets this, clear otherwise */
  FLAG  speaker_on;	     /* added by MSF - TRUE - voice on, else off */
};
static struct sb_dsp_type dsp;  /* 'dsp' structure used everywhere */

#define NUM_UNITS 1

/* This will be used as a global holder for the current Sound Blaster
   general status values. */
struct sb_status_type
{
  FLAG  dsp_in_use;	     /* DSP or MIDI open for reading or writing */
  FLAG  fm_in_use;	      /* FM open for ioctls */
  FLAG  cms_in_use;	     /* CMS open for ioctls */

  FLAG  alive;		  /* Card present? */
  unsigned int addr;	    /* Sound Blaster card address */
  unsigned int irq;	     /* MSF-added this */
  int  *wake[NUM_UNITS];	/* What to wakeup on interrupt */
};
static struct sb_status_type status; /* Global current status */


/*
 * Forward declarations galore!
 */
int sb_probe (struct sb_conf *dev);
int sb_attach (struct sb_conf *dev);
void sb_unattach (void);
void sb_sendb (unsigned select_addr, BYTE reg,
		      unsigned data_addr, BYTE value);
int dsp_reset (void);
int dsp_open (struct sb_conf *dev);
int dsp_close (void);
unsigned int dsp_set_speed (unsigned int *speed);
int dsp_command (BYTE val);
int dsp_set_voice (int on);
 int dsp_flush_dac (void);
 int dsp_set_compression (int mode);
 int dsp_set_stereo (FLAG on);
 int dsp_write (char *ptr, int len);
 int dsp_read (BYTE far *ptr, int len);
 void dsp_find_buffers (void);
 void dsp_dma_start (int dir);
 void dsp_next_write (void);
 void dsp_next_read (void);
 void mixer_send (BYTE reg, BYTE val);
 BYTE mixer_read_reg (BYTE reg);
 int mixer_reset (void);
 int mixer_set_levels (struct sb_mixer_levels *l);
 int mixer_set_params (struct sb_mixer_params *p);
 int mixer_get_levels (struct sb_mixer_levels *l);
 int mixer_get_params (struct sb_mixer_params *params);

// From mouse.h
extern unsigned eventButtons;
extern unsigned LastEventButtons;

/* added by msf */
/* old_irqptr holds the old vector for the SB IRQ while we use it here.
 *  The IRQ vector is restored to this value when the program terminates */
void interrupt (* old_irqptr)(...);
void interrupt (* old_keyptr)(...);

/* this is how on declares a function to be an interrupt handler under
 *  MSC 5.1.   I know Borland compilers have their own way of handling this */
void interrupt far sb_intr (...);
void interrupt far key_intr (void);
void outb( int, int );

/*======================================================================*/
/* WVN - no 'timeout' on interrupt wait... press a key to abort program */
void wait_for_int (void)
{

/* wait for SB interrupt, or key hit */
while (!kbhit() && !DMA_complete &&
		 !(LastEventButtons & LEFTBUTTON && !(eventButtons & LEFTBUTTON))) ;
if (!DMA_complete)
	{
	/* must of been a key hit */
	DPRINTF(("DMA not complete!!!\n"));
	dsp_set_voice (OFF);
	dsp_reset();

//	exit (0);
	/* 'atexit' will take care of resetting SB interrupt */
	}
else
	DPRINTF(("DMA complete!\n"));

DMA_complete = 0;
}

/*======================================================================*/
/*
 * Probing sets dev->dev_alive, status.alive, and returns 1 if the
 * card is detected.  Note that we are not using the "official" Adlib
 * of checking for the presence of the card because it's tacky and
 * takes too much mucking about.  We just attempt to reset the DSP and
 * assume that a DSP-READY signal means the card is there.  If you
 * have another card that fools this probe, I'd really like to hear
 * about it. :-) Future versions may query the DSP version number
 * instead.
 */
int
sb_probe (struct sb_conf *dev)
{

char *pEnv;

/* Check for the BLASTER environment variable to read hardware values */
/* ...the user can enter hardware values at the command line....*/
/* ...if not, then BLASTER will be read...*/
/* ...if neither of the above, defaults will be applied...*/

if ( (pEnv = getenv("BLASTER")) != NULL ) {
    while ( *pEnv ) {
      switch (*pEnv) {

	case 'A':		   /* read address (up to space) */
	case 'a':
	  if (dev->addr == 0)
		{
		sscanf( ++pEnv,"%x", &(dev->addr) ); /* remember, addr is in HEX ! */
		}
	  ++pEnv;
	  break;

	case 'I':		   /* read IRQ address (up to space) */
	case 'i':
	  if (dev->irq == 0)
		{
		dev->irq  = atoi( ++pEnv );   /* irq in decimal */
		}
	  ++pEnv;
	  break;

	case 'D':		       /* DMA channel */
	case 'd':
	  if (SB_DMA_CHAN < 0 )
		{
		SB_DMA_CHAN = atoi( ++pEnv );
		}
	  ++pEnv;
	  break;

	default:			/* ignore everything else */
	  pEnv++;
      }

    } /* while */
  } /* if */

/* make sure values are valid -- use defaults at slightest provocation */

if ( dev->addr == 0 )
	dev->addr = 0x220;
if ( dev->irq == 0 )
	dev->irq = 7;

if ( SB_DMA_CHAN < 0)
	SB_DMA_CHAN = 1;

if      ( SB_DMA_CHAN == 0 )
	DMA_PAGE = 0x87;
else if ( SB_DMA_CHAN == 1 )
	DMA_PAGE = 0x83;
else if ( SB_DMA_CHAN == 2 )
	DMA_PAGE = 0x81;
else if ( SB_DMA_CHAN == 3 )
	DMA_PAGE = 0x82;
else
	{
	status.alive = 0;
	printf ("Invalid DMA channel: %d\n",SB_DMA_CHAN);
	return (status.alive);
	}

#ifdef FULL_DEBUG
  printf ("sb: sb_probe() called.\n");
  printf ("Dev.addr = %d, dev.irq= %d DMA chan %d\n",
		 dev->addr, dev->irq, SB_DMA_CHAN);
#endif

status.addr = (unsigned int) dev->addr;

if (dsp_reset () == GOOD)
    status.alive = 1;
else
    {
    status.alive = 0;
    printf ("Sound Blaster Not found!  Driver not installed.\n");
	 }

return (status.alive);

}

/*======================================================================*/
/* MSF - This should be called when done, resets interrupts and stuff
 *       Machine can act unpredictably otherwise.		     */
void
sb_unattach (void)
{
unsigned int  tm;

DPRINTF(("sb_unattach:In sb_unattach\n"));
hfree(memory_chunk);

/* Return interrupts */
_disable();

if ( old_irqptr != NULL )
	{
	DPRINTF(("sb_unattach:Resetting vector back to original\n"));
	if (status.irq < 8)
		_dos_setvect(8+status.irq, old_irqptr);
	else
		_dos_setvect(0x70+(status.irq & 0x07), old_irqptr);
	}

/* Mask DSP interrupt */
if (status.irq < 8)
	{
	tm = inp(0x21);
	outb(0x21,tm | (1 << status.irq));
	}
else
	{
	tm = inp(0xa1);
	outb(0xa1,tm | (1 << (status.irq & 0x07) ));
	}

_enable();

}

/*=======================================================================*/
/* WVN - new interrupt routine... keep it as short as possible	   */
void interrupt far sb_intr (...)
{
DMA_complete = 1;
inp (DSP_RDAVAIL);	  /* ack interrupt */
if (status.irq < 8)
	outb(0x20,0x20);
else
	outb(0x20,0x20);
	outb(0xa0,0x20);
}

/*======================================================================*/
/* MSF - supposed to wait ten microseconds, although I havent timed it
 *       Probably waits longer, important thing is that you cant write
 *       to the DSP without a delay between writes		    */
void tenmicrosec(void)
{
long i;
// Was 18
for (i = 0; i < 9; i++) inp(0x388);
}

/*==================================================================*/
/*
 * Send one byte to the named Sound Blaster register.
 * The SBDK recommends 3.3 microsecs after an address write,
 * and 23 after a data write.  What they don't tell you is that
 * you also can't READ from the ports too soon, or crash! (Same timing?)
 * Anyway, 10 usecs is as close as we can get, so..
 *
 * NOTE:  This function is wicked.  It ties up the entire machine for
 * over forty microseconds.  This is unacceptable, but I'm not sure how
 * to make it better.  Look for a re-write in future versions.
 * Does 30 microsecs merit a full timeout() proceedure?
 */
 void
sb_sendb (unsigned int select_addr, BYTE reg,
	  unsigned int data_addr, BYTE value)
{
  outb (select_addr, reg);
  tenmicrosec();
  outb (data_addr, value);
  tenmicrosec();
  tenmicrosec();
  tenmicrosec();
}

/*====================================================================*/
/*
 * DSP functions.
 */
/*
 * Reset the DSP chip, and initialize all DSP variables back
 * to square one.  This can be done at any time to abort
 * a transfer and break out of locked modes. (Like MIDI UART mode!)
 * Note that resetting the DSP puts the speed back to 8196, but
 * it shouldn't matter because we set the speed in dsp_open.
 * Keep this in mind, though, if you use DSP_IOCTL_RESET from
 * inside a program.
 */
int
dsp_reset (void)
{
  int i;

  DPRINTF (("dsp_reset:Resetting DSP.\n"));
  dsp.used[0] = dsp.used[1] = 0; /* This is only for write; see dsp_read() */
  dsp.full[0] = dsp.full[1] = FALSE;
  dsp.hi = dsp.active = 0;
  dsp.first_block = 1;
  dsp.error = ESUCCESS;
  dsp.cont_xfer = NULL;
  status.wake[DSP_UNIT] = NULL;

  /*
   * This is how you reset the DSP, according to the SBDK:
   * Send 0x01 to DSP_RESET (0x226) and wait for three microseconds.
   * Then send a 0x00 to the same port.
	* Poll until DSP_RDAVAIL's most significant bit is set, indicating
   * data ready, then read a byte from DSP_RDDATA.  It should be 0xAA.
   * Allow 100 microseconds for the reset.
   */
  tenmicrosec();		/* Lets things settle down. (necessary?) */
  outb (DSP_RESET, 0x01);
  tenmicrosec();
  outb (DSP_RESET, 0x00);

  dsp.error = EIO;
  for (i = DSP_LOOP_MAX; i; i--)
    {
      tenmicrosec();
      if ((inp (DSP_RDAVAIL) & DSP_DATA_AVAIL)
	  && ((inp (DSP_RDDATA) & 0xFF) == DSP_READY))
	{
	  dsp.error = ESUCCESS;
	  break;
	}
	 }
  if (dsp.error != ESUCCESS)
    return (FAIL);
  else
    return (GOOD);
}

/*======================================================================*/

/* WVN - changed it to 64K buffers
*/

/*
 * MSF - following was changed to deal with 64k segments of MSC.
 *       Follows idea of original code, just looks uglier.
 *       I grab 32k and look for 2 10K buffers in that space which
 *       dont cross a 64k boundary.
 *
 * ORIGINAL COMMENTS (msf)
 *
 * This finds page-safe buffers for the DSP DMA to use.  A single DMA
 * transfer can never cross a 64K page boundary, so we have to get
 * aligned buffers for DMA.  The current method is wasteful, but
 * allows any buffer size up to the full 64K (which is nice).  We grab
 * 3 * 64K in the static global memory_chunk, and find the first 64K
 * alignment in it for the first buffer.  The second buffer starts 64K
 * later, at the next alignment.  Yeah, it's gross, but it's flexible.
 * I'm certainly open to ideas!  (Using cool kernel memory alloc is tricky
 * and not real portable.)
 */
 void
dsp_find_buffers (void)
{
unsigned long startseg, startoff, break64k, allocation;

//if (DSP_BUF_SIZE > (1L << 16) - 1 )
//	{
//	printf ("DMA Buffer must be less than 64K\n");
//	exit(1);
//	}

//if (DSP_BUF_SIZE > 1L << 15)
	allocation = 2L * DSP_BUF_SIZE + (1L << 16);
//else
//	allocation = 3L * DSP_BUF_SIZE;
DPRINTF (("DSP_BUF_SIZE %ld allocation %ld\n",DSP_BUF_SIZE,allocation));

if ( !(memory_chunk =  (char huge*)halloc ( allocation, 1 ) ) )
	{
	printf ("DMA buffer allocation failed\n");
	exit(1);
	}
DPRINTF (("memory_chunk %8.8lx\n", memory_chunk));

startseg = (unsigned long) ((unsigned long)memory_chunk & 0xffff0000L) >> 16;
startoff = (unsigned long)		 memory_chunk & 0x0000ffff;
DPRINTF (("startseg %8.8lx startoff %8.8lx\n",startseg,startoff));

break64k = ((((startseg*16 + startoff) >> 16) + 1) << 16);
DPRINTF (("break64k %ld\n",break64k));

dsp.buf[0] = memory_chunk;
dsp.phys_buf[0] = startseg*16 + startoff;
DPRINTF (("buffer #0: %8.8lx %ld\n",dsp.buf[0],dsp.phys_buf[0]));

if (dsp.phys_buf[0] + DSP_BUF_SIZE > break64k)
	{
	dsp.buf[0] += (break64k - dsp.phys_buf[0]);
	dsp.phys_buf[0] = break64k;
	break64k += 1L << 16;
	DPRINTF (("Address changed to: %8.8lx %ld\n",dsp.buf[0],dsp.phys_buf[0]));
	}

dsp.buf[1] = dsp.buf[0] + DSP_BUF_SIZE;
dsp.phys_buf[1] = dsp.phys_buf[0] + DSP_BUF_SIZE;
DPRINTF (("buffer #1: %8.8lx %ld\n",dsp.buf[1],dsp.phys_buf[1]));

if (dsp.phys_buf[1] + DSP_BUF_SIZE > break64k)
	{
	dsp.buf[1] += (break64k - dsp.phys_buf[1]);
	dsp.phys_buf[1] = break64k;
	DPRINTF (("Address changed to: %8.8lx %ld\n",dsp.buf[1],dsp.phys_buf[1]));
	}
}

/*========================================================================*/

void dsp_dma_start (void)
{

unsigned int count = dsp.used[dsp.active] - 1;

/* Prepare the DMAC.  See sb_regs for defs and more info. */

/* MSF - this is real stupid, but MSC optimization makes writes too fast
	  with inline code, so calling dummy function outb() slows it
	  down enough ! */

outb (DMA_MASK_REG, DMA_MASK);
outb (DMA_CLEAR, 0);
outb (DMA_MODE, DMA_MODE_WRITE);
outb (DMA_PAGE,    (int)((dsp.phys_buf[dsp.active] & 0xff0000L) >> 16)); /* Page */
outb (DMA_ADDRESS, (int)(dsp.phys_buf[dsp.active] & 0x00ff)); /* LSB of address */
outb (DMA_ADDRESS, (int)((dsp.phys_buf[dsp.active] & 0xff00) >> 8));
outb (DMA_COUNT, count & 0x00ff);
outb (DMA_COUNT, (count & 0xff00) >> 8);
outb (DMA_MASK_REG, DMA_UNMASK);

DPRINTF(("dsp_dma_start:Sent DMA instructions for length %u\n",count));

/*
 * The DMAC is ready, now send the commands to the DSP.
 * Notice that there are two entirely different operations for
 * Low and High speed DSP.  With HS, You only have to send the
 * byte-count when it changes, and that requires an extra command
 * (Checking if it's a new size is quicker than always sending it.)
 */

if (dsp.hispeed)
	{
	DPRINTF (("Starting High-Speed DMA of %u bytes to/from buffer %d.\n",
		dsp.used[dsp.active], dsp.active));

	if (count != DSP_BUF_SIZE - 1)
		{
		dsp_command (HIGH_SPEED_SIZE);
		dsp_command ((BYTE) (count & 0x00ff));
		dsp_command ((BYTE)((count & 0xff00) >> 8));
		}
	dsp_command (dsp.start_command); /* GO! */
	}
else			  /* Low Speed transfer */
	{
	DPRINTF (("dsp_dma_start:Starting Low-Speed DMA xfer of %u bytes to/from buffer %d.\n",
		dsp.used[dsp.active], dsp.active));
	dsp_command (dsp.start_command);
	dsp_command ((BYTE) (count & 0x00ff));
	dsp_command ((BYTE)((count & 0xff00) >> 8)); /* GO! */
	}

}

/*=======================================================================*/
/* WVN - this changed a little, because the wait_for_int routine changed */

int dsp_write (char *ptr, int len)
{
unsigned int bytecount, hunk_size;

/* MSF - if speaker off turn it on */
if (!dsp.speaker_on) dsp_set_voice(ON);

if (dsp.first_block)
	{
	dsp.first_block = FALSE;
	}

bytecount = 0;

/* While there is still data to write, and data in this chunk.. */
while (bytecount < len)
	{
	hunk_size = min(len - bytecount, DSP_BUF_SIZE - dsp.used[dsp.hi]);
	DPRINTF (("dsp_write:Adding %u bytes (%u) to buffer %d.\n",
		  hunk_size, dsp.used[dsp.hi], dsp.hi));
	memmove(dsp.buf[dsp.hi]+dsp.used[dsp.hi], ptr, hunk_size);
	dsp.used[dsp.hi] += hunk_size;
	ptr += hunk_size;

	if (dsp.used[dsp.hi] == DSP_BUF_SIZE)
		{
		dsp.full[dsp.hi] = TRUE;
		DPRINTF (("dsp_write:Just finished filling buffer %d.\n", dsp.hi));
		DPRINTF (("dsp_write:Waiting for buffer %d to empty.\n", dsp.active));
		dsp.active = dsp.hi;
		wait_for_int();

		dsp_dma_start();

		dsp.hi ^= 1;      /* Switch to other buffer */
		dsp.used[dsp.hi] = 0; /* Mark it as empty */

		}		   /* if filled hi buffer */

	bytecount += hunk_size;
	}		       /* While there are bytes left in chunk */

return (dsp.error);
}

/*=======================================================================*/
/* WVN - this will be called by dsp_close */

/*
 * MSF - Call this if you want to make sure everything got written out.
 *
 * Play any bytes in the last waiting write() buffer and wait
 * for all buffers to finish transferring.
 * An even number of bytes is forced to keep the stereo channels
 * straight.  I don't think you'll miss one sample.
 */
int dsp_flush_dac (void)
{
DPRINTF (("dsp_flush_dac:Flushing last buffer(s).\n"));
//wait_for_int();
//for (i=0; i<32000; i++) ;
DMA_complete = 0;

if (dsp.used[dsp.hi] != 0)
	{
	DPRINTF (("dsp_flush_dac:Playing the last %u bytes.\n", dsp.used[dsp.hi]));
	dsp.full[dsp.hi] = TRUE;
	dsp.active = dsp.hi;
	dsp_dma_start();
	wait_for_int();
	}

return (ESUCCESS);
}

/*========================================================================*/
int dsp_open (struct sb_conf *dev)
{
unsigned int i;
unsigned int im, tm;

if (!sb_probe (dev)) return (0);

/* MSF - added irq field to status */
status.addr = (unsigned int) dev->addr;
status.irq =  (unsigned int) dev->irq;
status.fm_in_use = 0;
status.dsp_in_use = 0;
status.addr = (unsigned int) dev->addr;
status.irq =  (unsigned int) dev->irq;

DPRINTF(("dsp_open:status.addr,status.irq= %x %x\n",status.addr,status.irq));
DPRINTF(("dsp_open:dev.addr,dev.irq= %x %x\n",dev->addr,dev->irq));

dsp_find_buffers();
for (i = 0; i < NUM_UNITS; i++)
	status.wake[i] = NULL;

/*
 * These are default startup settings.
 */
dsp.compression = PCM_8;
dsp.speed = 11025;

status.dsp_in_use = TRUE;
dsp_reset ();		 /* Resets card and inits variables */
dsp_set_speed (&dsp.speed);   /* Set SB back to the current speed. */
dsp_set_voice(OFF);    /* turn off speaker by default */

#ifdef FULL_DEBUG
/* Stuff buffers with loud garbage so we can hear/see leaks. */
for (i = 0; i < DSP_BUF_SIZE; i++)
	dsp.buf[0][i] = dsp.buf[1][i] = i & 0xff;
#endif

/* Enable interrupts on PIC */
if (dev->irq < 8)
	{
	im = inp(0x21);
	tm = ~(1 << dev->irq);
	outb(0x21,im & tm);
	_enable();
	outb(0x20,0x20);
	}
else
	{
	im = inp(0xa1);
	tm = ~(1 << (dev->irq & 0x07) );
	outb(0xa1,im & tm);
	_enable();
	outb(0x20,0x20);
	outb(0xa0,0x20);
	}

DPRINTF(("dsp_open:Interrupt mask register = %4.4x\n",im));
DPRINTF(("dsp_open:Setup IRQ %d with mask %4.4x\n",dev->irq, im & tm));

/* Set up DSP interrupt */
if (dev->irq < 8)
	{
	old_irqptr = _dos_getvect(8+dev->irq);
	_dos_setvect(8+dev->irq, sb_intr);
	}
else
	{
	old_irqptr = _dos_getvect(0x70+(dev->irq & 0x07) );
	_dos_setvect(0x70+(dev->irq & 0x07), sb_intr);
	}
_enable();

DPRINTF (("dsp_open:Sound Blaster installed. (Port 0x%X, IRQ %d, DMA %d)\n",
	  status.addr, dev->irq, SB_DMA_CHAN));

/* MSF - set things up so if program exits it will call sb_unattach */
//if (atexit( sb_unattach ))
//	printf("Error calling atexit\n");

return (1);

}

/*========================================================================*/

int dsp_close (void)
{
  if (status.dsp_in_use)
	 {
		/* Wait for any last write buffers to empty  */
		dsp_flush_dac ();
		dsp_set_voice(OFF);
/*      dsp_reset (); */
		status.dsp_in_use = FALSE;
		return (ESUCCESS);
	 }
  else
	 return (ESRCH);	     /* Does this ever happen? */
}

/*========================================================================*/
/*
 * Set the playback/recording speed of the DSP.
 * This takes a pointer to an integer between DSP_MIN_SPEED
 * and DSP_MAX_SPEED and changes that value to the actual speed
 * you got. (Since the speed is so darn granular.)
 * This also sets the dsp.hispeed flag appropriately.
 * Note that Hi-Speed and compression are mutually exclusive!
 * I also don't check all the different range limits that
 * compression imposes.  Supposedly, the DSP can't play compressed
 * data as fast, but that's your problem.  It will just run slower.
 * Hmmm.. that could cause interrupt timeouts, I suppose.
 */
 /* Changed by MSF to use unsigned int, still pass -1 (65535) to query speed
 */
unsigned int dsp_set_speed (unsigned int *speed) {
   BYTE time_constant;

// if (*speed == -1)
//    {
//      *speed = dsp.speed;
//      return (ESUCCESS);
//    }

//  if (*speed < (long)DSP_MIN_SPEED || *speed > (long)DSP_MAX_SPEED)
//    {
//      DPRINTF (("Attempt to set invalid speed (%ud)\n", *speed));
//      return (EINVAL);
//    }
	if (*speed < (long)DSP_MIN_SPEED)
	{
		DPRINTF(("Speed too low! (%d)\n", *speed));
		return (EINVAL);
	}
	else if (*speed > (long)DSP_MAX_SPEED)
	{
		DPRINTF(("Speed too high! (%d)\n", *speed));
		return (EINVAL);
	}

DPRINTF (("Speed starts at %d.\n", *speed));
time_constant = (BYTE) ((65536L - (256000000L / (long)*speed)) >> 8);
DPRINTF (("Time constant is %d.\n", time_constant));
dsp.speed = (256000000L / (65536L - ((long)time_constant << 8)));
*speed = dsp.speed;
DPRINTF (("Speed set to %d.\n", dsp.speed));

if (*speed > MAX_LOW_SPEED)
	{
	if (dsp.compression != PCM_8)
		return (EINVAL);
	DPRINTF (("Using HiSpeed mode.\n"));
	dsp.start_command = HS_DAC_8;
	dsp.hispeed = TRUE;
	}
else
	{
	DPRINTF (("Using LowSpeed mode.\n"));
	dsp.start_command = DAC_8;
	dsp.hispeed = FALSE;
	}

/* Here is where we actually set the card's speed */
if (dsp_command (SET_TIME_CONSTANT) == FAIL
      || dsp_command (time_constant) == FAIL)
	return (EIO);

/*
 * In case we do any High-Speed transfers, this sets the transfer
 * size.  We only need to set it when it changes.  See dsp_dma_start()
 */
if (*speed > MAX_LOW_SPEED)
	{
	dsp_command (HIGH_SPEED_SIZE);
	dsp_command ((DSP_BUF_SIZE - 1) & 0x00ff);
	dsp_command (((DSP_BUF_SIZE - 1) & 0xff00) >> 8);
	}

return (ESUCCESS);
}

/*=======================================================================*/
/*
 * Turn the DSP output speaker on and off.
 * Argument of zero turns it off, on otherwise
 */
 int
dsp_set_voice (int on)
{
  if (dsp_command ( (BYTE)(on ? SPEAKER_ON : SPEAKER_OFF) ) == GOOD)
   {
    dsp.speaker_on = on ? TRUE : FALSE;
    return (ESUCCESS);
   }
  else
    return (EIO);
}

/*========================================================================*/
/*
 * Send a command byte to the DSP port.
 * First poll the DSP_STATUS port until the BUSY bit clears,
 * then send the byte to the DSP_COMMAND port.
 */
 int
dsp_command (BYTE val)
{
  int i;

#ifdef FULL_DEBUG
  printf ("dsp_command:Sending DSP command 0x%X\n", val);
#endif
  for (i = DSP_LOOP_MAX; i; i--)
    {
      if ((inp (DSP_STATUS) & DSP_BUSY) == 0)
	{
	  outb(DSP_COMMAND, val);
	  return (GOOD);
	}
      tenmicrosec ();
    }
  printf ("sb: dsp_command (%2X) failed!\n", val);
  return (FAIL);
}
/*====================================================================*/
/* MSF - added because I was sending stuff to the DMA registers too
 *       fast when MSC put in inline out dx,ax commands. This subroutine
 *       slows things down, maybe too much. Suggestions welcome. */
void outb(int x, int y)
{
inp(0x388);
outp(x,y);
}
