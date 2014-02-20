/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/
#define INCL_KBD
#include <os2.h>
#include "16bit.h"

USHORT _THUNK_FUNCTION (Dos16GetInfoSeg) (PSEL pselGlobal, PSEL pselLocal);

void DosGetInfoSeg(PSEL pselGlobal, PSEL pselLocal) {
  PVOID16 pselG16, pselL16;
  
  pselG16 = _emx_32to16(pselGlobal);
  pselL16 = _emx_32to16(pselLocal);
  _THUNK_PROLOG(8);
  _THUNK_FAR16(pselG16);
  _THUNK_FAR16(pselL16);
  _THUNK_CALL(Dos16GetInfoSeg);
}
