/*======================================================================

   Header file for Sound Blaster user programs.

   [ This file is part of the SBMSDOS v1.0 distibution ]

   Michael Fulbright (msf@as.arizona.edu)

   This file was originally distributed by the fellow below, I'm
   just borrowing it.

   =====================================================================
   ORIGINAL HEADER FOLLOWS (msf)
   =====================================================================

   [ This file is a part of SBlast-BSD-1.4 ]

   Steve Haehnichen <shaehnic@ucsd.edu>

   $Id: sblast.h,v 1.5 1992/06/13 01:48:15 steve Exp steve $
 

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

 * $Log: sblast.h,v $
 * Revision 1.5  1992/06/13  01:48:15  steve
 * Released in SBlast-BSD-1.4

======================================================================*/

#ifndef SBLAST_H
#define SBLAST_H

typedef unsigned char BYTE;
typedef unsigned char FLAG;

/*
 * Available compression modes
 * (These are actually DSP DMA-start commands, but you don't care)
 */
enum
{
  PCM_8         = 0x14,         /* Straight 8-bit PCM */
  ADPCM_4       = 0x74,         /* 4-bit ADPCM compression */
  ADPCM_2_6     = 0x76,         /* 2.5-bit ADPCM compression */
  ADPCM_2       = 0x16,         /* 2-bit ADPCM compression */
};

//enum { LEFT, RIGHT, BOTH };     /* Stereo channel choices */

/* Mixer: Set mixer volume levels
 *        Set mixer control parameters (non-volume stuff)
 *        Read current mixer volume levels
 *        Read current mixer parameters
 *        Reset the mixer to a default state
 */       
#define MIXER_IOCTL_SET_LEVELS  _IOW('s', 20, struct sb_mixer_levels)
#define MIXER_IOCTL_SET_PARAMS  _IOW('s', 21, struct sb_mixer_params)
#define MIXER_IOCTL_READ_LEVELS _IOR('s', 22, struct sb_mixer_levels)
#define MIXER_IOCTL_READ_PARAMS _IOR('s', 23, struct sb_mixer_params)
#define MIXER_IOCTL_RESET       _IO('s', 24)

/* DSP: Reset the DSP and terminate any transfers.
 *      Set the speed (in Hz) of DSP playback/record.
 *      (Note that the speed parameter is modified to be the actual speed.)
 *      Turn the DSP voice output on (non-zero) or off (0)
 *      Flush any pending written data.
 *      Set the DSP decompression mode to one of the above modes.
 *      Set Stereo playback/record on (non-zero) or off (0)
 */
#define DSP_IOCTL_RESET         _IO('s', 0)
#define DSP_IOCTL_SPEED         _IOWR('s', 1, int)
#define DSP_IOCTL_VOICE         _IOW('s', 2, FLAG)
#define DSP_IOCTL_FLUSH         _IO('s', 3)
#define DSP_IOCTL_COMPRESS      _IOW('s', 4, BYTE)
#define DSP_IOCTL_STEREO        _IOW('s', 5, FLAG) /* Can go to mixer too */

/* DSP legal speed range (really!) */
#define         DSP_MAX_SPEED   47619L
#define         DSP_MIN_SPEED   3906L

struct stereo_vol
{
  BYTE l;                       /* Left volume */
  BYTE r;                       /* Right volume */
};


/*
 * Mixer volume levels for MIXER_IOCTL_SET_VOL & MIXER_IOCTL_READ_VOL
 */
struct sb_mixer_levels
{
  struct stereo_vol master;     /* Master volume */
  struct stereo_vol voc;        /* DSP Voice volume */
  struct stereo_vol fm;         /* FM volume */
  struct stereo_vol line;       /* Line-in volume */
  BYTE cd;                      /* CD audio */
  BYTE mic;                     /* Microphone level */
};

/*
 * Mixer parameters for MIXER_IOCTL_SET_PARAMS & MIXER_IOCTL_READ_PARAMS
 */
struct sb_mixer_params
{
  BYTE record_source;           /* Recording source (See SRC_xxx below) */
  FLAG hifreq_filter;           /* Filter frequency (hi/low) */
  FLAG filter_input;            /* ANFI input filter */
  FLAG filter_output;           /* DNFI output filter */
  FLAG dsp_stereo;              /* 1 if DSP is in Stereo mode */
};
#define SRC_MIC         1       /* Select Microphone recording source */
#define SRC_CD          3       /* Select CD recording source */
#define SRC_LINE        7       /* Use Line-in for recording source */

#define ESUCCESS 0
#define ERANGE   1
#define EIO      2
#define EINVAL   3
#define ESRCH    4



#endif                          /* !def SBLAST_H */
