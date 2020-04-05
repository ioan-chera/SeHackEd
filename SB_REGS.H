/*======================================================================

   [ This file is part of the SBMSDOS v1.0 distibution ]

   Michael Fulbright (msf@as.arizona.edu)

   This file was originally distributed by the fellow below, I'm
   just borrowing it.

   =====================================================================
   ORIGINAL HEADER FOLLOWS (msf)
   =====================================================================

   Huge collection of SoundBlaster registers, flags, and such.
   [ This file is a part of SBlast-BSD-1.4 ]

   Steve Haehnichen <shaehnic@ucsd.edu>

   $Id: sb_regs.h,v 1.9 1992/06/13 01:47:23 steve Exp steve $

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

 * $Log: sb_regs.h,v $
 * Revision 1.9  1992/06/13  01:47:23  steve
 * Released in SBlast-BSD-1.4

======================================================================*/

/*
 * This is where you set the Sound Blaster DMA channel.
 * Possible settings are 0, 1, and 3.  The card defaults to
 * channel 1, which works just fine for me.
 *
 * The IRQ and I/O Port must be set in autoconf.c to match your card
 * settings.  See the directions in INSTALL for details.
 */
int SB_DMA_CHAN = -1;

/* Convenient byte masks */
#define B1(x)   ((x) & 0x01)
#define B2(x)   ((x) & 0x03)
#define B3(x)   ((x) & 0x07)
#define B4(x)   ((x) & 0x0f)
#define B5(x)   ((x) & 0x1f)
#define B6(x)   ((x) & 0x3f)
#define B7(x)   ((x) & 0x7f)
#define B8(x)   ((x) & 0xff)
#define F(x)    (!!(x))     /* 0 or 1 only */

/*
 * DMA registers & values. (Intel 8237: Direct Memory Access Controller)
 */

/* Mask register: Send DMA_MASK to this register to suspend DMA on that
 * channel, then DMA_UNMASK to allow transfers.  Generally, you
 * send DMA_MASK as the first thing in the DMA setup, then UNMASK it last */
#define DMA_MASK_REG    0x0A    /* Mask register */
#define DMA_UNMASK      SB_DMA_CHAN
#define DMA_MASK    (DMA_UNMASK | 0x04)

/* The DMA_MODE register selects a Read or Write DMA transfer. */
#define DMA_MODE    0x0B    /* Mode register (read/write to card) */
#define DMA_MODE_WRITE  (0x48 + SB_DMA_CHAN) /* DAC */
#define DMA_MODE_READ   (0x44 + SB_DMA_CHAN) /* ADC */

/* This is a strange register.  Basically you just have to send
 * a byte to it before writing the address.  (It sets the address
 * state to LSB.)  Always send a 0x00 or anything else before
 * writing the address. */
#define DMA_CLEAR       0x0C /* Send one byte before writing the address. */

/* This is where you send the address of the block of memory involved
 * in the transfer.  This block may start anywhere in the Page, but
 * the Count must not cross a 64K page boundary.   This is very important!
 * Write 2 bytes (low byte first, followed by the high byte) for the
 * least-significant 16 bits of the address. */
#define DMA_ADDRESS     (SB_DMA_CHAN << 1)

/* Write 2 bytes, low byte first for the 16 bits of the count.
   Note that one greater than this value will be copied!
   (i.e. the biggest size is 0xFFFF, and 0x0000 would transfer one byte. */
#define DMA_COUNT       (DMA_ADDRESS + 1)

/* The DMA_PAGE register gets bits 16-23 of the address.
 * Combined with the LSB and MSB sent to the address register,
 * This allows for a 16Meg addressing range.
 * Note that the Page registers are not in any logical order, so you
 * just have to pick the right one out of some table. */
int DMA_PAGE;

/*  ---- Sound Blaster DSP values ---
 *
 * These are the card addresses where data may be read or written.
 * status.addr is the base address of the card, as defined by
 * SB_IO_PORT above.  (I use status.addr in the kernel for flexibilty.)
 * The typical settings are 0x220 or 0x240  (0x220 is the default)
 * See the docs for an explanation of each register.
 */
#define DSP_RESET           (status.addr + 0x06) /* Pulse to reset DSP */
#define DSP_RDDATA          (status.addr + 0x0A) /* Read data */
#define DSP_WRDATA          (status.addr + 0x0C) /* Write data */
#define DSP_COMMAND         (status.addr + 0x0C) /* Send command */
#define DSP_STATUS          (status.addr + 0x0C) /* Read status */
#define DSP_RDAVAIL         (status.addr + 0x0E) /* Data available */

/* Status bits:
 * These are bits within the named readable registers that mean something. */
#define DSP_BUSY        (1 << 7) /* STATUS flag indicates not ready */
#define DSP_DATA_AVAIL      (1 << 7) /* RDAVAIL flag for data ready */
#define DSP_READY           0xAA     /* RDDATA generated by a good reset */

/* DSP commands:
 * These are the possible commands to send to the DSP via the DSP_COMMAND
 * register.  See the docs for more detailed explanations. */
#define SET_TIME_CONSTANT       0x40 /* Set the sampling rate (1 byte) */
#define SPEAKER_ON          0xD1 /* Turn on DSP sound */
#define SPEAKER_OFF         0xD3 /* Mute DSP voice output */
#define READ_SPEAKER        0xD8 /* Read speaker status (00:OFF FF:ON) */
#define HALT_DMA        0xD0 /* Pause the DMA transfer */
#define CONT_DMA        0xD4 /* Continue paused DMA transfer */
#define GET_VERSION         0xE1 /* Get the DSP major & minor versions */

#define HIGH_SPEED_SIZE     0x48 /* Set the HS block size. (LSB & MSB) */
#define HS_DAC_8        0x91 /* Start High-Speed 8-bit DMA DAC */
#define HS_ADC_8        0x99 /* Start High-Speed 8-bit DMA ADC */

#define SILENCE         0x80 /* Send a block of silence */
#define DIRECT_DAC_8        0x10 /* For sending a single sample byte */
#define DIRECT_ADC_8        0x20 /* For sampling a single byte */

/* Commands to start Low-Speed DMA transfers.
   For the decompression modes, add one to get the command for
   data with reference byte (first block of sample) */
#define DAC_8           0x14 /* Start 8-bit DMA DAC */
#define DAC_4           0x74 /* Start 4-bit DMA DAC */
#define DAC_2_6         0x76 /* Start 2.6-bit DMA DAC */
#define DAC_2           0x16 /* Start 2-bit DMA DAC */
#define ADC_8           0x24 /* Start 8-bit DMA ADC */

/* MIDI DSP commands. */
#define MIDI_POLL           0x30 /* Poll a byte from the MIDI port */
#define MIDI_READ           0x31 /* Initiate a MIDI read with interrupts */
#define MIDI_UART_POLL      0x34 /* Poll for a byte while allowing write */
#define MIDI_UART_READ      0x35 /* Initiate a read while allowing write */
#define MIDI_WRITE          0x38 /* Write a single MIDI byte */


/*
 * Sound Blaster on-board Mixer chip addresses:
 */
#define MIXER_ADDR          (status.addr + 0x04)
#define MIXER_DATA          (status.addr + 0x05)

/* To reset the Mixer to power-up state, send a 0x00 to both the
 * ADDR and DATA ports.  (Actually, you can send any value to DATA after
 * sending 0x00 to the ADDR port.) */
#define MIXER_RESET         0x00 /* Send this to both ADDR & DATA */

/*
 * Mixer chip registers:
 */
#define RECORD_SRC      0x0C
/*  bit:  7 6 5 4 3 2 1 0       F=frequency (0=low, 1=high)
      x x T x F S S x       SS=source (00=MIC, 01=CD, 11=LINE)
                    T=input filter switch (ANFI) */

/* Recording sources (SRC_MIC, SRC_CD, SRC_LINE)
   are defined in i386at/sblast.h */

#define IN_FILTER       0x0C    /* "ANFI" analog input filter reg */
#define OUT_FILTER      0x0E    /* "DNFI" output filter reg */
#define FREQ_HI     (1 << 3) /* Use High-frequency ANFI filters */
#define FREQ_LOW    0       /* Use Low-frequency ANFI filters */
#define FILT_ON     0       /* Yes, 0 to turn it on, 1 for off */
#define FILT_OFF    (1 << 5)

#define CHANNELS    0x0E    /* Stereo/Mono output select */
#define MONO_DAC    0       /* Send to OUT_FILTER for mono output */
#define STEREO_DAC      2       /* Send to OUT_FILTER for stereo output */

#define VOL_MASTER      0x22    /* High nibble is left, low is right */
#define VOL_FM      0x26
#define VOL_CD      0x28
#define VOL_LINE    0x2E
#define VOL_VOC     0x04
#define VOL_MIC     0x0A    /* Only the lowest three bits. (0-7) */
