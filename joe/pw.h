/*
 *	Prompt windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Prompt window (a BW) */

struct pw {
	int	(*pfunc) ();	/* Func which gets called when RTN is hit */
	int	(*abrt) ();	/* Func which gets called when window is aborted */
	int	(*tab) ();	/* Func which gets called when TAB is hit */
	char	*prompt;	/* Prompt string */
	int	promptlen;	/* Width of prompt string */
	int	promptofst;	/* Prompt scroll offset */
	B	*hist;		/* History buffer */
	void	*object;	/* Object */
	int	file_prompt;	/* Set if this is a file name prompt, so do ~ expansion */
};

/* BW *wmkpw(BW *bw,char *prompt,int (*func)(),char *huh,int (*abrt)(),
             int (*tab)(),void *object,int *notify);
 * Create a prompt window for the given window
 * file_prompt flags:
 *   bit 0: ~ expansion
 *   bit 1: update directory
 *   bit 2: seed with directory
 */
BW *wmkpw(W *w, char *prompt, B **history, int (*func) (), char *huh, int (*abrt) (), int (*tab) (), void *object, int *notify, struct charmap *map, int file_prompt);

int ucmplt(BW *bw, int k);

/* Function for TAB completion */

char **regsub(char **z, int len, char *s);

void cmplt_ins(BW *bw,char *line);

int cmplt_abrt(BW *bw,int x, char *line);

int cmplt_rtn(MENU *m,int x,char *line);

int simple_cmplt(BW *bw,char **list);

void setup_history(B **history);
void append_history(B *hist,char *s,int len);
void promote_history(B *hist, off_t line);
void set_current_dir(BW *bw, char *s,int simp);

extern int bg_prompt;
extern int nocurdir;

extern WATOM watompw;

char *get_cd(W *w);
