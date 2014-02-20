/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/

typedef ULONG PVOID16;
  
typedef struct _GINFOSEG { 
        ULONG   time; ULONG   msecs; UCHAR   hour;
        UCHAR   minutes; UCHAR   seconds; UCHAR   hundredths;
        USHORT  timezone; USHORT cusecTimerInterval; UCHAR   day;
        UCHAR   month; USHORT  year; UCHAR   weekday;
        UCHAR   uchMajorVersion; UCHAR uchMinorVersion;
        UCHAR   chRevisionLetter; UCHAR   sgCurrent;
        UCHAR   sgMax; UCHAR   cHugeShift;
        UCHAR   fProtectModeOnly; USHORT pidForeground;
        UCHAR   fDynamicSched; UCHAR   csecMaxWait;
        USHORT  cmsecMinSlice; USHORT  cmsecMaxSlice;
        USHORT  bootdrive; UCHAR   amecRAS[32];
        UCHAR   csgWindowableVioMax; UCHAR   csgPMMax;
} GINFOSEG;
typedef GINFOSEG *PGINFOSEG;

typedef struct _keypacket {
  USHORT     mnflags;
  KBDKEYINFO cp;
  USHORT     ddflags;
} KEYPACKET;

void DosGetInfoSeg(PSEL pselGlobal, PSEL pselLocal);



