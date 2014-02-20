/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/
#define INCL_WIN
#include <os2.h>
#include <time.h>
#include "blackout.h"

HWND hwndFrameShared;
int MouseMovesShared;
time_t LastTimeBlackCalledShared;

BOOL EXPENTRY InputHook(HAB hab, PQMSG pQmsg, ULONG fs) {
  static time_t LastTimePosted = 0; 
  time_t temptime;

  if (pQmsg->msg == WM_CHAR || pQmsg->msg == WM_BUTTON1DOWN ||
      pQmsg->msg == WM_BUTTON2DOWN || pQmsg->msg == WM_BUTTON3DOWN ||
      pQmsg->msg == WM_MOUSEMOVE && MouseMovesShared)
    if (time(&temptime) - LastTimePosted > 3
	&& temptime - LastTimeBlackCalledShared > 8) {
      LastTimePosted = temptime;
      WinPostMsg(hwndFrameShared, WM_USER, 0, 0);
    }
  return 0;
}

/*
BOOL EXPENTRY InputHook(HAB hab, PQMSG pQmsg, ULONG fs) {
  if (pQmsg->msg == WM_MOUSEMOVE)
    DosBeep(100, 100);
  return 0;
}
*/




