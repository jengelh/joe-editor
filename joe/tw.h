/*
 *	Text editing windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Text window (a BW) */

struct tw {
	char	*stalin;	/* Status line info */
	char	*staright;
	int	staon;		/* Set if status line was on */
	off_t	prevline;	/* Previous cursor line number */
	int	changed;	/* Previous changed value */
	B	*prev_b;	/* Previous buffer (we need to update status line on nbuf/pbuf) */
};

BW *wmktw(Screen *t, B *b);

int usplitw(BW *bw);
int uduptw(BW *bw);
int utw0(BASE *b);
int utw1(BASE *b);
int uabortbuf(BW *bw);
int ucancel(BW *bw, int k);
int upopabort(BW *bw);
int uabort(BW *bw, int k);
int uabort1(BW *bw, int k);
void setline(B *b, off_t line);
int abortit(BW *bw);
extern int staen;
extern int staupd;
extern int keepup;
extern int bg_stalin;

extern WATOM watomtw;
char *stagen(char *stalin, BW *bw, char *s, int fill);
