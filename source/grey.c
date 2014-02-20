/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/
#define INCL_WIN
#define INCL_DOSPROCESS
#include <stdlib.h>
#include <os2.h>
#include <graph.h>
#include <sys/hw.h>
#include "vga.h"
#include <stdio.h>

#define MAX_PAL_SIZE 256

extern void ErrMsg(char *s);

static unsigned char newtable[MAX_PAL_SIZE * 3], oldtable[MAX_PAL_SIZE * 3];

/*
  void setpalette(char *p) {
  _portaccess(FIRST_PORT, LAST_PORT);  
  _wait01(IS1_R, 0x08);
  _outp8(PEL_IW, 0);
  _outps8dac(PEL_D, p, 3 * 256);
  }
  */

void setpalette(char *p) {                          
  g_vgapal(p, 0, MAX_PAL_SIZE, 1);          
}                                                   


int checkpalettemgr(HWND hwnd) {
  HAB hab;
  HDC hdc;
  ULONG caps;

  hab = WinQueryAnchorBlock(hwnd);
  hdc = WinOpenWindowDC(hwnd);

  DevQueryCaps(hdc, CAPS_ADDITIONAL_GRAPHICS, 1, &caps);
  if (caps & CAPS_PALETTE_MANAGER)
    return 1;
  else
    return 0;
}
                                         
/*
void getpalette(char *p) {
   int i, j;

  _portaccess(FIRST_PORT, LAST_PORT);
  _outp8(PEL_IR, 0);
  for(j = 0; j < 256 * 3; j++) {
    for(i = 0; i < 20; i++) ;
    *p++ = _inp8(PEL_D);
  }
}
*/

void getpalette(char *p) {               
  int i;                                  
  
  _portaccess(FIRST_PORT, LAST_PORT);     
  for (i = 0; i < 256; i++) {         
    _outp8(PEL_IR, i);
    *p++ = _inp8(PEL_D);                 
    *p++ = _inp8(PEL_D);                 
    *p++ = _inp8(PEL_D);                 
  }                                       
}                                        

void grey_blank(HWND hwnd) {
  int i;

  getpalette(oldtable);
  for (i = 0; i < 256 * 3; i++) 
    newtable[i] = oldtable[i] / 2;
  setpalette(newtable);

  WinShowPointer(HWND_DESKTOP, 0);
}

void grey_unblank(HWND hwnd) {
  setpalette(oldtable);
  WinShowPointer(HWND_DESKTOP, 1);
}















