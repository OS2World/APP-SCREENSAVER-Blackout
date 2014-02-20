/*=======================================================================
This file is part of the Blackout screen saver.          v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.  See README file for details.
========================================================================*/
/* This is the code that performs dpms blanking functions in Blackout.
   It was ported by Staffan Ulfberg from the console drivers in Linux
   on the 4th of April 1995.  You can see where the Linux people got
   the code from in the comments below.  The Linux code is almost
   unchanged, except for that I have added a main() function for use
   with Blankout.  All changes in the rest of the code are commented.
   You can search for "OS/2" to find the relevant changes.  I have
   also deleted a few include lines as well as a few external
   declarations.  Thanks to Henrik Storner for helping me find the
   original source code.

   And - just like the comments below say: You use this code at your
   own risk.  I take NO RESPONSIBILITY WHATSOEVER for damage caused by
   using the code below! */


/* -------------- ADDED WHEN PORTING THIS CODE TO OS/2 ------------- */
#define INCL_DOS
#define INCL_KBD
#define INCL_MOU
#define INCL_VIO
#define INCL_WINSWITCHLIST
#include <stdlib.h>
#include <sys/hw.h> 
#include <stdio.h>
#include <os2.h>
#include <string.h>
#include "vga.h"
#include "16bit.h"
#define video_port_reg  CRT_I
#define video_port_val  CRT_D
#define outb_p(a,b) _outp8(b,a)
#define inb_p(a) _inp8(a)
#define cli()
#define sti()

static void vesa_blank(void);
static void vesa_unblank(void);
static void set_vesa_blanking(const unsigned long arg);

HEV hevSuspend, hevOff, hevWait;

VIOCURSORINFO viocursorinfo = { 0, 0, 0, -1 };

int main(int argc, char *argv[]) {
  USHORT rc, mask = 126;
  UCHAR status;
  HMOU hmou;

  if (argc != 3 || strcmp(argv[1], "0") && strcmp(argv[1], "1")) {
    printf("Usage: %s mouse dpms.\n", argv[0]);
    printf("\nmouse is '0' or '1' for mouse move sensing off or on.\n");
    printf("dpms is '1' to use DPMS functions, '0' just clears the screen.\n");
    sleep(5);
    exit(1);
  }
  if (rc = DosOpenEventSem("\\SEM32\\BLACKOUT\\SUSPEND", &hevSuspend)) {
    printf("DosOpenEventSem(Suspend): %d\nBlackout is not running?\n", rc);
    sleep(5);
    exit(2);
  }
  if (rc = DosOpenEventSem("\\SEM32\\BLACKOUT\\WAIT", &hevWait)) {
    printf("DosOpenEventSem(Wait): %d\nBlackout is not running?\n", rc);
    sleep(5);
    exit(3);
  }
  if (rc = DosOpenEventSem("\\SEM32\\BLACKOUT\\OFF", &hevOff)) {
    printf("DosOpenEventSem(Off): %d\nBlackout is not running?\n", rc);
    sleep(5);
    exit(5);
  }
  MouOpen(NULL, &hmou);
  if (*argv[1] == '1')
    mask++;
  MouSetEventMask(&mask, hmou);
  VioSetCurType(&viocursorinfo, 0);
  
  sleep(2);
  VioScrLock(0, &status, 0);
  if (status) {
    printf("Can't lock screen!\n");
    sleep(5);
    exit(4);
  }

  if (*argv[2] == '1') {
    set_vesa_blanking(1);
    vesa_blank();
    DosWaitEventSem(hevSuspend, (ULONG)SEM_INDEFINITE_WAIT);
    vesa_unblank();
    DosWaitEventSem(hevWait, (ULONG)SEM_INDEFINITE_WAIT);
    set_vesa_blanking(2);
    vesa_blank();
    DosWaitEventSem(hevOff, (ULONG)SEM_INDEFINITE_WAIT);
    vesa_unblank();
  } else
    DosWaitEventSem(hevOff, (ULONG)SEM_INDEFINITE_WAIT);
  
  VioScrUnLock(0);
  MouClose(hmou);
  DosCloseEventSem(hevSuspend);
  DosCloseEventSem(hevOff);
  DosCloseEventSem(hevWait);
  return 0;
}

/* ------------------------ END OS/2 CHANGE ------------------------ */


/*
 * vesa_blank.c
 *
 * Exported functions:
 *	void vesa_blank(void);
 *	void vesa_unblank(void);
 *	void set_vesa_blanking(const unsigned long arg);
 *
 * Not all hardware reacts well to this code - activate at your own risk.
 * Activation is done using a sufficiently recent version of setterm
 * or using a tiny C program like the following.
 *
-----------------------------------------------------------------------
|#include <stdio.h>
|#include <linux/termios.h>
|main(int argc, char *argv[]) {
|    int fd;
|    struct { char ten, onoff; } arg;
|
|    if (argc != 2) {
|	fprintf(stderr, "usage: setvesablank ON|on|off\n");
|	exit(1);
|    }
|    if ((fd = open("/dev/console", 0)) < 0)
|      fd = 0;
|    arg.ten = 10;
|    arg.onoff = 0;
|    if (!strcmp(argv[1], "on"))
|      arg.onoff = 1;
|    else if (!strcmp(argv[1], "ON"))
|      arg.onoff = 2;
|    if (ioctl(fd, TIOCLINUX, &arg)) {
|	perror("setvesablank: TIOCLINUX");
|	exit(1);
|    }
|    exit(0);
|}
-----------------------------------------------------------------------
*/


/*
 * This file handles the VESA Power Saving Protocol that lets a
 * monitor be powered down whenever not needed for a longer time.
 * The VESA protocol defines:
 *
 *  Mode/Status		HSync	VSync	Video
 *  -------------------------------------------
 *  "On"		on	on	active  (mode 0)
 *  "Suspend" {either}	on	off	blank   (mode 1)
 *            {  or  }	off	on	blank   
 *  "Off"               off	off	blank	(mode 2)
 *
 * Original code taken from the Power Management Utility (PMU) of
 * Huang shi chao, delivered together with many new monitor models
 * capable of the VESA Power Saving Protocol.
 *
 * Adapted to Linux by Christoph Rimek (chrimek@toppoint.de)  15-may-94.
 * A slightly adapted fragment of his README follows.
 *
Patch (based on Linux Kernel revision 1.0) for handling the Power Saving
feature of the new monitor generation. The code works on all these monitors
(mine is a Smile 1506) and should run on *all* video adapter cards (change
some i/o-addresses), although tested only on two different VGA-cards: a  
cheap Cirrus Logic (5428) and a miro Crystal 8S (S3-805).

You can choose from two options:

(1) Setting vesa_blanking_mode to 1.
    The code will save the current setting of your video adapters'
    register settings and then program the controller to turn off
    the vertical synchronisation pulse.

(2) Setting vesa_blanking_mode to 2.
    If your monitor locally has an Off_Mode timer then you should not
    force your video card to send the OFF-signal - your monitor will
    power down by itself.
    If your monitor cannot handle this and needs the Off-signal directly,
    or if you like your monitor to power down immediately when the
    blank_timer times out, then you choose this option.

On the other hand I'd recommend to not choose this second option unless
it is absolutely necessary. Powering down a monitor to the Off_State with
an approx. power consumption of 3-5 Watts is a rather strong action for
the CRT and it should not be done every now and then. If the software only  
sends the signal to enter Standby mode, you have the chance to interfere
before the monitor powers down. Do not set a too short period, if you love
your hardware :-)) .

If requested, in the future it may be possible to install another timer
to provide a configurable delay between the two stages Standby and Off
similar to the "setterm -blank"-feature.
*/

#define seq_port_reg	(0x3c4)		/* Sequencer register select port */
#define seq_port_val	(0x3c5)		/* Sequencer register value port  */
#define video_misc_rd	(0x3cc)		/* Video misc. read port	  */
#define video_misc_wr	(0x3c2)		/* Video misc. write port	  */


/* structure holding original VGA register settings */
static struct {
	unsigned char	SeqCtrlIndex;		/* Sequencer Index reg.   */
	unsigned char	CrtCtrlIndex;		/* CRT-Contr. Index reg.  */
	unsigned char	CrtMiscIO;		/* Miscellaneous register */
	unsigned char	HorizontalTotal;	/* CRT-Controller:00h */
	unsigned char	HorizDisplayEnd;	/* CRT-Controller:01h */
	unsigned char	StartHorizRetrace;	/* CRT-Controller:04h */
	unsigned char	EndHorizRetrace;	/* CRT-Controller:05h */
	unsigned char	Overflow;		/* CRT-Controller:07h */
	unsigned char	StartVertRetrace;	/* CRT-Controller:10h */
	unsigned char	EndVertRetrace;		/* CRT-Controller:11h */
	unsigned char	ModeControl;		/* CRT-Controller:17h */
	unsigned char	ClockingMode;		/* Seq-Controller:01h */
} vga;

static int vesa_blanking_mode = 0;
static int vesa_blanked = 0;

/* routine to blank a vesa screen */
void vesa_blank(void)
{
        int mode;

	if((mode = vesa_blanking_mode) == 0)
	  return;

	_portaccess(FIRST_PORT, LAST_PORT);  /* Added OS/2 */

	/* save original values of VGA controller registers */
	cli();
	vga.SeqCtrlIndex = inb_p(seq_port_reg);
	vga.CrtCtrlIndex = inb_p(video_port_reg);
	vga.CrtMiscIO = inb_p(video_misc_rd);
/*	sti(); OS/2 */

	if(mode == 2) { 
	    outb_p(0x00,video_port_reg);		/* HorizontalTotal */
	    vga.HorizontalTotal = inb_p(video_port_val);
	    outb_p(0x01,video_port_reg);		/* HorizDisplayEnd */
	    vga.HorizDisplayEnd = inb_p(video_port_val);
	    outb_p(0x04,video_port_reg);		/* StartHorizRetrace */
	    vga.StartHorizRetrace = inb_p(video_port_val);
	    outb_p(0x05,video_port_reg);		/* EndHorizRetrace */
	    vga.EndHorizRetrace = inb_p(video_port_val);
	}
	outb_p(0x07,video_port_reg);			/* Overflow */
	vga.Overflow = inb_p(video_port_val);
	outb_p(0x10,video_port_reg);			/* StartVertRetrace */
	vga.StartVertRetrace = inb_p(video_port_val);
	outb_p(0x11,video_port_reg);			/* EndVertRetrace */
	vga.EndVertRetrace = inb_p(video_port_val);
	outb_p(0x17,video_port_reg);			/* ModeControl */
	vga.ModeControl = inb_p(video_port_val);
	outb_p(0x01,seq_port_reg);			/* ClockingMode */
	vga.ClockingMode = inb_p(seq_port_val);
	
	/* assure that video is enabled */
	/* "0x20" is VIDEO_ENABLE_bit in register 01 of sequencer */
/*	cli();  OS/2 */
	outb_p(0x01,seq_port_reg);
	outb_p(vga.ClockingMode | 0x20,seq_port_val);

	/* test for vertical retrace in process.... */
	if ((vga.CrtMiscIO & 0x80) == 0x80)
		outb_p(vga.CrtMiscIO & 0xef,video_misc_wr);

	/*
	 * Set <End of vertical retrace> to minimum (0) and
	 * <Start of vertical Retrace> to maximum (incl. overflow)
	 * Result: turn off vertical sync (VSync) pulse.
	 */

	outb_p(0x10,video_port_reg);		/* StartVertRetrace */
	outb_p(0xff,video_port_val);		/* maximum value */
	outb_p(0x11,video_port_reg);		/* EndVertRetrace */
	outb_p(0x40,video_port_val);            /* minimum (bits 0..3)  */
	outb_p(0x07,video_port_reg);		/* Overflow */
	outb_p(vga.Overflow | 0x84,video_port_val);	/* bits 9,10 of  */
	/* vert. retrace */
      
	if (mode == 2) {
	    /*
	     * Set <End of horizontal retrace> to minimum (0) and
	     *  <Start of horizontal Retrace> to maximum
	     * Result: turn off horizontal sync (HSync) pulse.
	     */
	    outb_p(0x04,video_port_reg);	/* StartHorizRetrace */
	    outb_p(0xff,video_port_val);	/* maximum */
	    outb_p(0x05,video_port_reg);	/* EndHorizRetrace */
	    outb_p(0x00,video_port_val);	/* minimum (0) */
	}

	/* restore both index registers */
	outb_p(vga.SeqCtrlIndex,seq_port_reg);
	outb_p(vga.CrtCtrlIndex,video_port_reg);
	sti();

	vesa_blanked = mode;
}	

/* routine to unblank a vesa screen */
void vesa_unblank(void)
{
	if (!vesa_blanked)
	  return;

	_portaccess(FIRST_PORT, LAST_PORT);  /* Added OS/2 */

	/* restore original values of VGA controller registers */
	cli();
	outb_p(vga.CrtMiscIO,video_misc_wr);

	if (vesa_blanked == 2) { 
	    outb_p(0x00,video_port_reg);	/* HorizontalTotal */
	    outb_p(vga.HorizontalTotal,video_port_val);
	    outb_p(0x01,video_port_reg);	/* HorizDisplayEnd */
	    outb_p(vga.HorizDisplayEnd,video_port_val);
	    outb_p(0x04,video_port_reg);	/* StartHorizRetrace */
	    outb_p(vga.StartHorizRetrace,video_port_val);
	    outb_p(0x05,video_port_reg);	/* EndHorizRetrace */
	    outb_p(vga.EndHorizRetrace,video_port_val);
	}
	outb_p(0x07,video_port_reg);		/* Overflow */
	outb_p(vga.Overflow,video_port_val);
	outb_p(0x10,video_port_reg);		/* StartVertRetrace */
	outb_p(vga.StartVertRetrace,video_port_val);
	outb_p(0x11,video_port_reg);		/* EndVertRetrace */
	outb_p(vga.EndVertRetrace,video_port_val);
	outb_p(0x17,video_port_reg);		/* ModeControl */
	outb_p(vga.ModeControl,video_port_val);
	outb_p(0x01,seq_port_reg);		/* ClockingMode */
	outb_p(vga.ClockingMode,seq_port_val);
	

	/* restore index/control registers */
	outb_p(vga.SeqCtrlIndex,seq_port_reg);
	outb_p(vga.CrtCtrlIndex,video_port_reg);
	sti();

	vesa_blanked = 0;
}

void set_vesa_blanking(const unsigned long arg)
{
/*	char *argp = (char *)(arg + 1); */
	unsigned int mode = /* get_fs_byte(argp) */ arg;  /* OS/2 CHANGE*/
	vesa_blanking_mode = ((mode < 3) ? mode : 0);
}
