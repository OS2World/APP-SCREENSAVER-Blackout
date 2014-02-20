/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/
#define INCL_WIN
#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_KBD
#define INCL_DOSMONITORS
#include <stdlib.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <os2.h>
#include <sys/wait.h>
#include <time.h>
#include "blackout.h"
#include "grey.h"
#include "16bit.h"

#define BLANKED_NOTBLANKED 0
#define BLANKED_GREY 1
#define BLANKED_SUSPEND 2
#define BLANKED_OFF 3

#define MSECS_PER_INTERVAL 10000
#define INTERVALS_PER_MIN 6

void start_monitor(ULONG);
void ErrMsg(char *);

static MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);

static long TimeBeforeGrey = 15;
static long TimeBeforeSuspend = 45;
static long TimeBeforeOff = 120; 
static long TimeAfterSuspendBeforeOff = 0;
static int MouseMovesBlack = 0, MouseMovesDulled = 0;
static int UseDPMS = 1;
static int BlankFullScreen = 1;
time_t *LastTimeBlackCalledShared;

void keyboard_thread(void *);
ULONG progtype(PID idSession);
void full_thread(void *);
void switchtopm(void);

HAB hab;
HEV hevSuspend, hevOff, hevWait;
HWND hwndFrame;
static int blanked = BLANKED_NOTBLANKED;

static PGINFOSEG gdt;

int main(int argc, char *argv[]) {
  HMQ hmq;
  ULONG flFrameFlags;
  HWND hwndClient, *hwndFrameShared;
  QMSG qmsg;
  HMODULE DllHandle;
  char DllLoadError[100];
  PFN DllProcAddr;
  int Remove = 0, InMem, AddToTaskList = 0, TimeoutsToSend = 0;
  int *MouseMovesShared, i;
  SEL gdt_descriptor, ldt_descriptor;
    
  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);

  DosGetInfoSeg(&gdt_descriptor, &ldt_descriptor);
  gdt = (PGINFOSEG)MAKEP(gdt_descriptor, 0);

  WinRegisterClass(hab, "Blackout.Win", ClientWndProc,
		   CS_SIZEREDRAW, 0L);

  if (argc == 2 && !strcmp(argv[1], "-r"))
    Remove = 1;
  else {
    for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-l")) 
	AddToTaskList = 1;
      else if (!strcmp(argv[i], "-g")) 
	TimeoutsToSend++;
      else if (!strcmp(argv[i], "-m")) 
	MouseMovesBlack = MouseMovesDulled = 1;
      else if (!strcmp(argv[i], "-o")) 
	MouseMovesDulled = 1;
      else if (!strcmp(argv[i], "-n")) 
	UseDPMS = 0;
      else if (!strcmp(argv[i], "-f")) 
	BlankFullScreen = 0;
      else {
	if (i == argc - 1)
	  ErrMsg("Bad command line options");
	if (!strcmp(argv[i], "-t")) 
	  TimeBeforeGrey = atol(argv[i+1]) < 1 ? 1 : atol(argv[i+1]);
	else if (!strcmp(argv[i], "-s")) 
	  TimeBeforeSuspend = atol(argv[i+1]);
	else if (!strcmp(argv[i], "-b")) 
	  TimeBeforeOff = atol(argv[i+1]);
	else if (!strcmp(argv[i], "-p")) 
	  TimeAfterSuspendBeforeOff = atol(argv[i+1]);
	else
	  ErrMsg("Bad command line option");
	i++;
      }
    }
  }

  flFrameFlags = (FCF_TITLEBAR      | FCF_SYSMENU |
                  FCF_SHELLPOSITION | FCF_ICON |
                  (FCF_TASKLIST * AddToTaskList) | FCF_MINBUTTON);
  
  hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0, &flFrameFlags,
				 "Blackout.Win", "Blackout Window", 0L, 0,
				 ID_BLACKOUT, &hwndClient);

  InMem = !DosQueryModuleHandle(DLL_NAME, &DllHandle);
  if (InMem && !Remove && !TimeoutsToSend)
    ErrMsg("Already loaded!");
  else if (Remove && !InMem)
    ErrMsg("Can't find program in memory!");
  
  if (DosLoadModule(DllLoadError, 100, DLL_NAME, &DllHandle))
    ErrMsg(DllLoadError);
  if (DosQueryProcAddr(DllHandle,0, "InputHook", &DllProcAddr))
    ErrMsg("DosQueryProcAddr InputHook");
  if (DosQueryProcAddr(DllHandle,0, "hwndFrameShared", (PPFN)&hwndFrameShared))
    ErrMsg("DosQueryProcAddr hwndFrameShared");
  if (DosQueryProcAddr(DllHandle,0, "MouseMovesShared",
		       (PPFN)&MouseMovesShared))
    ErrMsg("DosQueryProcAddr MouseMovesShared"); 
  if (DosQueryProcAddr(DllHandle,0, "LastTimeBlackCalledShared",
		       (PPFN)&LastTimeBlackCalledShared))
    ErrMsg("DosQueryProcAddr LastTimeBlackCalledShared"); 
  
  if (Remove)
    WinPostMsg(*hwndFrameShared, WM_CLOSE, 0, 0);
  else if (TimeoutsToSend && InMem)
    while (TimeoutsToSend--)
      WinPostMsg(*hwndFrameShared, WM_FAKETIMER, 0, 0);      
  else {
    *hwndFrameShared = hwndFrame;
    *MouseMovesShared = MouseMovesDulled; 
    *LastTimeBlackCalledShared = 0; 
    if (!WinSetHook(hab, NULLHANDLE, HK_INPUT, DllProcAddr, DllHandle))
      ErrMsg("Winsethook");

    if (DosCreateEventSem("\\SEM32\\BLACKOUT\\SUSPEND", &hevSuspend,
			  DC_SEM_SHARED, FALSE))
      ErrMsg("DosCreateEventSem: \\SEM32\\BLACKOUT\\SUSPEND");
    if (DosCreateEventSem("\\SEM32\\BLACKOUT\\OFF", &hevOff,
			  DC_SEM_SHARED, FALSE))
      ErrMsg("DosCreateEventSem: \\SEM32\\BLACKOUT\\OFF");
    if (DosCreateEventSem("\\SEM32\\BLACKOUT\\WAIT", &hevWait,
			  DC_SEM_SHARED, FALSE))
      ErrMsg("DosCreateEventSem: \\SEM32\\BLACKOUT\\WAIT");

    if (_beginthread(full_thread, NULL, 10000l, 0) == -1)
      ErrMsg("_beginthread"); 

    while (TimeoutsToSend--)
      WinPostMsg(hwndFrame, WM_FAKETIMER, 0, 0);      

    while (WinGetMsg(hab, &qmsg, 0L, 0, 0))
      WinDispatchMsg(hab, &qmsg);
    
    WinReleaseHook(hab, NULLHANDLE, HK_INPUT, DllProcAddr, DllHandle);
    DosCloseEventSem(hevSuspend);
    DosCloseEventSem(hevOff);
    DosCloseEventSem(hevWait);
  }
  DosFreeModule(DllHandle);
  WinDestroyWindow(hwndFrame);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return (0);
}

static ULONG timer;

void settimer(ULONG new) {
  if ((timer = new) <= 0)
    WinPostMsg(hwndFrame, WM_FAKETIMER, 0, 0);
}

static MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg,
				      MPARAM mp1, MPARAM mp2) {
  static ULONG ptype;
  HPS hps;
  RECTL rcl;
  ULONG count;

  switch (msg) {
  case WM_CREATE:
    if (!checkpalettemgr(hwnd))
      TimeBeforeSuspend = 0;
    WinStartTimer(hab, hwnd, 0, MSECS_PER_INTERVAL);
    blanked = BLANKED_NOTBLANKED;
    settimer(TimeBeforeGrey * INTERVALS_PER_MIN);
    break;
  case WM_CLOSE:
    WinStopTimer(hab, hwnd, 0);
    WinPostMsg(hwnd, WM_QUIT, 0, 0);
    break;
  case WM_PAINT:
    hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
    WinQueryWindowRect(hwnd, &rcl);
    GpiSetColor(hps, CLR_DARKCYAN);
    WinDrawText(hps, -1, "Not much of a window is it?", &rcl, 0, 0,
		DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_ERASERECT);
    WinEndPaint(hps);
    break;
  case WM_TIMER:
    settimer(timer - 1);
    break;
  case WM_FAKETIMER:
    if (blanked == BLANKED_OFF)
      break;
    if (blanked == BLANKED_SUSPEND) {
      DosPostEventSem(hevSuspend);
      sleep(TimeAfterSuspendBeforeOff);
      DosPostEventSem(hevWait);
      blanked = BLANKED_OFF;
      break;
    }
    if ((ptype = progtype(gdt->sgCurrent)) == PROG_VDM)
      break;
    if (ptype == PROG_FULLSCREEN
	&& !BlankFullScreen && blanked == BLANKED_NOTBLANKED)
      break;
    if (blanked == BLANKED_NOTBLANKED && TimeBeforeSuspend) {
      grey_blank(hwnd);
      settimer(TimeBeforeSuspend * INTERVALS_PER_MIN);
      blanked = BLANKED_GREY;
    } else {
      if (blanked == BLANKED_GREY)
	grey_unblank(hwnd);
      if (ptype == PROG_FULLSCREEN)
	switchtopm();
      count = 0;
      DosResetEventSem(hevSuspend, &count);
      count = 0;
      DosResetEventSem(hevOff, &count);
      count = 0;
      DosResetEventSem(hevWait, &count);
      WinSetFocus(HWND_DESKTOP, hwndFrame);
      time(LastTimeBlackCalledShared);
      if (spawnl(P_SESSION, "black", "black", 
		 MouseMovesBlack ? "1" : "0", UseDPMS ? "1" : "0", NULL) == -1)
	ErrMsg("spawn(black.exe) failed");
      blanked = BLANKED_SUSPEND;
      settimer(TimeBeforeOff * INTERVALS_PER_MIN);
    } 
    break;
  case WM_USER:
    if (blanked == BLANKED_GREY) 
      grey_unblank(hwnd);
    else if (blanked == BLANKED_OFF || blanked == BLANKED_SUSPEND) {      
      DosPostEventSem(hevSuspend);
      DosPostEventSem(hevWait);
      DosPostEventSem(hevOff);
      if (waitpid(-1, NULL, WNOHANG) == -1)  /* Just eat up SIGCHLD. */
	ErrMsg("waitpid() error.");
    }
    blanked = BLANKED_NOTBLANKED;
    settimer(TimeBeforeGrey * INTERVALS_PER_MIN);
    break;
  default:
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
  }
  return 0;
}

void ErrMsg(char *s) {
  char t[100];
  
  if (!s) {
    _ultoa(WinGetLastError(hab), t, 10);
    s = t;
  }
  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, s, "Blackout", 0,
		MB_ERROR | MB_CANCEL);
  exit(1);
}

/* Function to return the type of programs in a session */
ULONG progtype(PID idSession) {
  USHORT num,i;
  SWBLOCK *pSwb;
  ULONG pt;

  if (!(num = WinQuerySwitchList(0L, NULL, 0)))
    ErrMsg("WinQuerySwitchList");

  pSwb = malloc(sizeof(HSWITCH) + sizeof(SWENTRY) * num);
  num = WinQuerySwitchList(0L, pSwb,
			   sizeof(HSWITCH) + sizeof(SWENTRY) * num);
  for(i = 0; i < num; i++) 
    if (pSwb->aswentry[i].swctl.idSession == idSession)
      pt = pSwb->aswentry[i].swctl.bProgType;

  free(pSwb);
  return pt;
}

/* Fuction to switch the PM screen to front */
void switchtopm(void) {
  SWBLOCK *pSwb;

  pSwb = malloc(sizeof(HSWITCH) + sizeof(SWENTRY));
  WinQuerySwitchList(0L, pSwb, sizeof(HSWITCH) + sizeof(SWENTRY));
  WinSwitchToProgram(pSwb->aswentry[0].hswitch);
  free(pSwb);
}


HKBD Handle[2];
MONIN *pmonInbuf[2];
MONOUT *pmonOutbuf[2];
KEYPACKET *pkeybuff[2];

void full_thread(void *slask) {
  UCHAR c, n;
  USHORT rc;
  int i;
  TID threadid[2];

  for (i = 0; i < 2; i++) {
    if (!(pmonInbuf[i] = _tcalloc(1, sizeof(MONIN))) ||
	!(pmonOutbuf[i] = _tcalloc(1, sizeof(MONOUT))) ||
	!(pkeybuff[i] = _tcalloc(1, sizeof(KEYPACKET))))
      ErrMsg("Could not allocate buffers.");
  }

  c = gdt->sgCurrent;
  for (;;) {
    for (i = 0; i < 2; i++) {
      if (rc = DosMonOpen(i ? "MOUSE$" : "KBD$", &Handle[i]))
	ErrMsg("DosMonOpen");
      DosCreateThread(threadid+i, &start_monitor, i, 0, 0x8000);  
      DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, threadid[i]);
    }
    while (c == (n = gdt->sgCurrent))
      sleep(5);
    c = n;
    for (i = 0; i < 2 ; i++) {
      DosKillThread(threadid[i]); 
      DosMonClose(Handle[i]);
    }
  } 
}

void start_monitor(ULONG device) {
  time_t LastTimePosted = 0, temptime, startuptime;
  USHORT count, rc;

  time(&startuptime);
  pmonInbuf[device]->cb  = sizeof(MONIN);
  pmonOutbuf[device]->cb = sizeof(MONOUT);

  if (rc = DosMonReg(Handle[device],
		     (PBYTE)pmonInbuf[device], (PBYTE)pmonOutbuf[device],
		     MONITOR_BEGIN, gdt->sgCurrent)) {
    DosExit(EXIT_THREAD, 1);
  }

  for (;;) {
    count = sizeof(KEYPACKET);
    DosMonRead((PBYTE)pmonInbuf[device], IO_WAIT,
	       (PBYTE)pkeybuff[device], &count);
    DosMonWrite((PBYTE)pmonOutbuf[device],
		(PBYTE)pkeybuff[device], count);

    if (time(&temptime) - LastTimePosted > 3 && (!device || temptime - startuptime > 8)) {
      LastTimePosted = temptime; 
      WinPostMsg(hwndFrame, WM_USER, 0, 0);
    } 
  }
}
