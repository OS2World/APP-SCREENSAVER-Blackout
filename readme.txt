======================================================================
This file is part of the Blackout screen saver.       v1.00 1996-06-29
Copyright (C) 1995, 1996 by Staffan Ulfberg.   
======================================================================


--- IMPORTANT NOTICE! --- READ THIS FRIST! ---

This program controls the graphics hardware of your computer.  Some
(but very few) graphics adapters are not supported, and serious bugs
may exist in this program.  One implication of this is that there is a
theoretical possibility that the program CAUSES DAMAGE to your system
if used.  If you don't want to take this risk, DO NOT RUN THE PROGRAM!
This program comes with absolutely NO WARRANTY.

Be sure to read the "Bugs / Future" and the "Copyright details"
sections of this document before installing, using, or distributing
this program.

----------------------------------------------



CONTENTS
========

1. What it is and what it is good for
2. Installation / Files in archive
3. Command line options
4. How it works
5. Bugs / Future
6. Credits
7. Copyright details
8. Author
9. Revision history



1. WHAT IT IS AND WHAT IT IS GOOD FOR
=====================================

"Blackout" is a completely FREE screen saver for OS/2 versions 2.0 and
above.  It is intended for use with monitors that have DPMS, or
"green," support.  I wrote the program primarily because I needed it
myself, and was unable to locate a free one.  At the same time I
thought this was a great time to learn about OS/2 programming.

The program blanks presentation manager and OS/2 full screen sessions;
DOS full screen sessions are not blanked.  Of course, this holds for
WIN-OS/2 full screen sessions as well.  I don't want my monitor to
switch back on only because I happen to shake the table; therefore,
the program is not sensitive to mouse moves by default.  A command
line switch can change this behaviour.

You can forget about fancy graphics.  This program has three levels of
screen saving: 1) after a user-programmable time, it dulls the screen.
This will save the monitor, but you will still be able to see that
it's turned on. 2) after some more time (possibly zero) the monitor
will be switched to suspend mode, and 3) the monitor is switched off
after some additional time.  Actually, it is also possible to replace
2) and 3) above with a turned on, black screen instead: look at the
command line options section.

If you can't see a difference between suspend and off modes, it's
probably because your monitor doesn't support suspend mode; some
monitors just don't.

If the screen dulling doesn't seem to work properly---start the
program using the switch "-s 0" to disable it (see Section 3); this is
the most common graphics adapter incompatibility problem.  The dull
(or grey) mode may work in some graphics modes but not in others.  In
some cases, these problems are recognized by Blackout and which then
adds the switch "-s 0" automatically.

There is now a homepage for Blackout on the world wide web; the URL is
"http://www.nada.kth.se/~staffanu/blackout/blackout.html".

Staffan Ulfberg
e-mail: staffanu@nada.kth.se



2. INSTALLATION / FILES IN ARCHIVE
==================================

First of all, you need to have the emx runtime environment installed.
If you don't already have that, get the file "emxrt.zip" which is
available at all the major OS/2 ftp sites.  Follow the installation
instructions that come with emxrt to install it.  Blackout won't run
without it.

Create a directory for the screen saver (e.g., "c:\blackout").  Unzip
the archive in this directory.  You will find the following files:

	README		This file
	COPYING		The GNU General Public Licence
	blackout.exe	The program to run to start Blackout.
	black.exe	Program to switch off VSYNC and HSYNC.  This
			program doesn't run alone without blackout
			running.
	blackdll.dll	DLL used by Blackout.
	source.zip	The source code for the program files above.

(Maybe you did this already:) The program to run is "blackout.exe."
It can be started from an icon on the desktop, possibly put in the
desktop startup folder if you want to start the program automatically
every time you start up your system.

The program won't say anything when started.  If you want to check if
the program is running you could 1) wait for the screen to be blanked,
2) use the -l option (see next section), or 3) try to start the
program again.  It will complain if already started.

One more thing: I strongly recommend not to start the "black.exe"
program when Blackout is running already.  (How many will try this
just because I told you not to???)



3. COMMAND LINE OPTIONS
=======================

On invocation, the following command line switches are available:

-t xx		Sets timeout before screen is greyed (made dull) in
		minutes.  The default setting is 15 minutes.

-s yy		Sets time after the screen becomes dull until it
		becomes switched to suspend mode using the DPMS
		function.  Default for this parameter is 45 minutes.

-b zz		Sets the time from that the screen is switched to
		suspend mode until it is turned off.  Default is 120
		minutes.  Think about that it is not good for your
		monitor to be turned off and back on too often!

-p ww		Some monitors and/or graphics adapters can't go
		directly from suspend to off mode.  With this option,
		the monitor is turned back on after suspend mode, and
		is switched off after ww seconds.  Default is 0.

-l		Add Blackout to the system window list, so it can be
		switched to by pressing CTRL-ESC.  The window
		displayed contains no information.  The only reason
		that this switch is included is that some people feel
		safer knowing how to end the program from the window
		list.

-m 		Make the program sensible to mouse moves.  By default
		only keyboard activity and mouse clicks are sensed
		for. 

-o		Like the -m switch, this also adds mouse move sensing,
		but only when the screen is on or dulled, and not when
		in suspend or off mode.

-n		Don't use DPMS saving; instead, just turn the screen
		black.  This switch is for people without DPMS
		monitors who still want a minimalistic screen saver.

-g		Send an activity timeout to the Blackout process.
		This option can be used to blank the screen at
		startup, or at any other time.  See discussion below.

-f		Tells Blackout not to blank full screen OS/2
		sessions.  Some full screen programs are reported not
		to work well with blackout; one of these is
		Microsoft's Programmer's Workbench for OS/2.

-r		Removes the program from memory if already loaded.


The diagram below illustrates the effect of the -t, -s, and -b
parameters.  "xx", "yy", and "zz" refers to the numeric arguments of
the respective switches.


 Screen  Screen         Screen          DPMS         DPMS     Screen
 is on   still on   greyed/dulled   suspend mode   off mode   back on
-------+----------+---------------+--------------+----------+-------->
 User    User                                                 User
 active  inactive                                             active

       <--- xx ---><----- yy -----><---- zz ----->


If you don't want the grey mode of saving, use the option "-s 0".
This is useful if your graphics adapter is not supported by blackout's
greying function.  However, in most such cases this option is
automatically added by Blackout.

The "-g" option can be used to blank the screen manually.  What will
actually happen is that a WM_TIMER message will be sent to the running
Blackout program---to make it think that the user has been inactive
for a long time.  This switch can be repeated several times on the
same command line: each occurence causes one WM_TIMER message to be
sent.  Normally, the first message will cause the screen to become
dull; the second will invoke suspend mode and the third off mode.  It
is possible to give this command line option when Blackout is first
started.  If Blackout is already running when invoked again with the
-g option, all other options are ignored.  So, make an icon with the
command line "-g -g" (or "-g -g -g" for off mode) to switch the screen
to suspend mode with a mouse click on the launchpad!


Some examples:

blackout -t 5 -b 0	Grey screen after 5 minutes, and switch
			monitor off after another 45 minutes.

blackout -t 30 -s 0	Switch monitor to suspend mode after 30
			minutes of inactivity, and then switch it off
			after another 120 minutes.



4. HOW IT WORKS
===============

In this section I'll try to explain a bit about the inner workings of
he program.  Since the source code is freely available - just unzip
the "source.zip" file that comes with this package - you can change it
yourself to fit your needs.  Still - DO read the copyright notice.

The following files are contained in the file "source.zip":

	Makefile 	Makefile to compile the program.

	blackout.def	Source files for the main program.
	blackout.rc
	blackout.ico
	blackout.c
	blackout.h
	grey.c
	grey.h

	blackdll.def	Source files for blackdll.dll.
	blackdll.c	

	black.c		Source file for black.exe.
	black.def

	16bit.c		THUNK code for 16 bit os calls.
	16bit.h

	vga.h		Contains constants for physical registers in
			the vga / vesa chips.


First of all, I should tell you that I use gcc/emx09b and gnumake to
compile the program.  You will also need sed to run my makefile (this
can be easily avoided, however).  I don't know much about other OS/2
compilers; if you use another compiler, I can't help you out with how
to port the program to it.

So, lets begin...  The main program is "blackout.c."  This is a PM
program.  On startup it installs an input hook to monitor keyboard and
mouse activity.  The hook function is in blackdll.c.  The input hook
sends messages "WM_USER" to the client window function when it detects
user activity.  To reduce the number of messages sent, the input hook
function never sends a message more often than every three seconds.

Every time the client window procedure receives a WM_USER message, it
resets a timer.  When the timer elapses, the screen is blanked.  If
the screen was already dulled, then DPMS blanking is started.  More
about this later.

A problem with the configuration above is that user input is never
detected in full screen sessions.  This problem is solved by two
mechanisms:

1) A thread which monitors the active session ID starts other threads,
which in turn install device monitors to monitor keyboard and mouse
activity in the current session.

2) The screen is never blanked if the current session is a DOS
full-screen session.  If this was not checked for, the screen would be
blanked when running DOS sessions as neither of the two user-activity
sensing mechanisms will not work.

The blanking function itself consists of two parts.  The first is
linked in the main program and is called when the screen should be
greyed.  The code currently writes directly to the VGA hardware.
Since this is a presentation manager application it should be possible
(?) to use the palette manager for this instead.  The truth is I don't
have the programming manuals for the Gpi functions---but some time in
the future the code in "grey.c" should be rewritten.

For the DPMS blanking, it is carried out by a full-screen executable
called black.exe.  The reason for putting this into a full-screen
program is that I write directly to the hardware.  I use the
VioScrLock function to be sure that no-one else can gain access to the
video hardware at the same time.  Once the screen is blanked, the
program waits for a semaphore which is cleared by the main program
(blackout.exe) when user activity starts again.

You might think that it is inconsistent to have the dpms functions in
a stand-alone executable when the grey functions are built in into the
program.  After all, both functions use the hardware directly.  You're
right.  It is inconsistent.  The reasons for doing things like this
are 1) the "grey" function doesn't switch the video mode.  Thus you
don't have to "hear" from the monitor every time the screen turns to
grey.  2) This code should eventually be changed so that it does not
write directly to the hardware.

So, I know the above is far from a complete description of the
program.  Hopefully it will be more complete in the future.  Please
read the following section.  There are lots of things to do...



5. BUGS / FUTURE
================

The sudden jump in version number to 1.00 is meant to indicate that
the functionality I expected from the program when I started working
on it is now there.  It's still far from perfect, and there may very
well be more versions to come.  However, it seems like the remaining
bugs would require a whole deal of work to correct, and it's unlikely
that I'll have the time to fix them in the near future.  Also, I
really hope that IBM will include functionality in future OS/2
versions to make Blackout redundant.

Below is a collection of known bugs and shortcomings.  If you fix a
bug or add to the program, please let me know so I can include the
changes in future versions.  If you're going to change something on
the list which is going to involve a lot of work---contact me first:
chances are it's already fixed.  Also, if you find a bug that's not on
the list, please let me know!


VGA/VESA hardware is assumed.  The port addresses are hard-wired in
the program and it will therefore not work with some adapters.  It
seems like the DPMS saving works with most color adapters, and also
that the grey function works when 256 colors are used but not
otherwise.

DOS full screen sessions are not blanked.  I think the easiest way to
solve this would be to have a DOS program started in the autoexec.bat
file for DOS sessions.  This program would simply be a DOS screen
saver program, but one that would notify blackout when screen saving
is needed instead of doing it itself.  I don't do DOS programming
myself, but I'd be happy to help whoever is interested in working on
this!

The program doesn't work correctly if the OS/2 Warp lockup function is
used.

The screen greying/dulling functions should be completely revised for
greater compatibility.  Currently the program assumes that 256 colors
are used and writes directly to the hardware.



6. CREDITS
==========

The code to switch off VSYNC and HSYNC is "stolen" from the Linux
console drivers.  Thanks to the Linux folks, and to Henrik Storner who
sent me the code.

I have studied the VESA drivers for emx written by Johannes Martin.
In fact, the file "vga.h" is a direct import from this package.

This software was developed using emacs, emx/gcc and gnumake.  Thanks
to the authors and contributors of the respective programs!

Thanks to Ethan Hall-Beyer for making a new good-looking icon for the
program!

D. J. Stauffer uploaded a file called "black2.zip" on hobbes some time
ago.  The contents was a replacement module for black.exe which just
turned the screen black instead of using DPMS functions.  I merged the
code into black.c and added the -n switch to select its behaviour.

Also, thanks to the following, who have pointed out bugs, tested debug
versions of the program, corrected bugs, or come up with interesting
ideas: Darren Abbott, Alan Benezra, Greg Bohn, Christoph Glanz, Tom
Maki, Ulf Loebich, Richard Solomon, and Dave Tholen.



7. COPYRIGHT DETAILS
====================

Copyright (C) 1995, 1996 by Staffan Ulfberg

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



8. AUTHOR
=========

The author of Blackout is

Staffan Ulfberg
V„stg”tagr„nd 5		(TeX: V\"astg\"otagr\"and)
118 28 Stockholm
Sweden

e-mail: staffanu@nada.kth.se



9. REVISION HISTORY
===================

11 April 1995	v0.9	First release of the program.

15 April 1995	v0.91   Added mouse button sensing in full screen
			sessions that use the mouse.  This makes it
			possible to exit black.exe by clicking the
			mouse.

			Makefile rewritten.

			The switch -s was added.

19 May 1995	v0.92	Added caption "Blackout" to error boxes.

			The switches -m and -g were added.

			The way that -s and -b work was changed.  (To
			make it possible to use both suspend and off
			mode.)

			The program icon was changed to a new
			better-looking one.

			"Full.exe" was rewritten to use threads
			instead of processes.  Much more efficient!

			The functions that grey the screen now read
			the original palette by Gpi calls; still have
			to do the writing in a clean way.

1 June 1995	v0.93	Full.exe was removed; its functionality was
			moved into blackout.exe.

			DPMS saving was added to OS/2 full-screen
			sessions.

			Automatic disabling of grey saving for most
			video modes where it's not supported.

1 October 1995	v0.94	Timer implementation changed to work with
			pre-Warp OS/2.  (Timeout was limited to
			65535ms on previous OS/2 versions.)

			Mouse move sensing code in blackdll changed
			slightly to overcome the problem with spurious
			mousemove events.  Result is that the -m
			switch works more reliably.

			The Gpi palette read introduced in v0.92 is
			gone again due to problems with it.

			The switches -p and -n were added.

29 June 1996 v1.00	Priority of threads that monitor full screen
			sessions changed to TIMECRITICAL.

			Mouse moves are not detected a few seconds
			after black.exe is started.  Should fix
			compatibility problems with some programs.

			The switches -f and -o were added.
