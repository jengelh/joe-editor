/*
 *	Editor startup and edit loop
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_MAIN_H
#define _JOE_MAIN_H 1

extern unsigned char *exmsg;	/* Exit message */
extern int help;		/* Set to start with help on */
extern Screen *maint;		/* Primary screen */
extern int usexmouse;		/* Use xterm mouse support? */
void nungetc(int c);
void dofollows(void);
int edloop(void);
void edupd(int flg);
extern KBD *shell_kbd;

extern volatile int dostaupd;	/* Force status line update */
extern int nonotice; /* Set to prevent copyright notice */
extern int noexmsg; /* Set to prevent final message */
extern int xmouse; /* XTerm mouse mode request by user (only allowed if terminal looks like xterm) */
extern unsigned char **mainenv; /* Environment variables passed to JOE */

extern unsigned char i_msg[128];
void internal_msg(unsigned char *);
void setlogerrs(void);
int ushowlog(BW *);

#endif
