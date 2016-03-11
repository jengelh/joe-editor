/*
 * Global object stack, plus dynamic string and dynamic array functions
 * built on top of them.
 *	Copyright
 *		(C) 2006 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* Object structure */

struct obj {
	LINK(Obj) link;		/* Memory dlist object is on */
	int size;		/* Alloc size of object */
	int len;		/* Length of object */
	union {
		double d;
		char c;
		int i;
		long l;
		void *p;
	} mem[1];		/* Data of the object, aligned properly */
};

extern Obj obj_stack[1];	/* The stack */

/* Get hidden header size */
#define obj_hdr_size ((char *)obj_stack->mem - (char *)obj_stack)

/* Get object base address from normal address */
#define obj_base(o) ((Obj *)((char *)(o) - obj_hdr_size))

/* Round up to min size, which should be a power of 2 */
#define round_up(n, to) (((n) + to) & ~(to - 1))

Obj get_obj_stack(); /* Get current stack, setup a new one in its place */

void set_obj_stack(Obj o); /* Restore a saved stack. */

/* Object memory allocation */

/* Free specified object.  If object is on the stack, all more recently
 * allocated objects are freed as well.  It's OK to call sfree(NULL). */

void obj_free(const void *ptr);

/* Allocate an object.  If you request zero bytes, a minimum sized block will
   be allocated.  The object is allocated on the stack. */

void *obj_malloc(int size);

/* Reallocate a block.  It's OK to request 0 bytes (a minimum sized block is
 * allocated). 'ptr' must be to a block already allocated with salloc(). 
 * Sreallocing a block does not change it's allocation order: sfree cares
 * about the order of salloc()s only, and does not care about srealloc(). 
 * */

void *obj_realloc(void *ptr, int new_size);

/* Turn object from stack allocated to heap allocated */

void obj_perm(const void *ptr);

/* Turn object from heap allocated to stack allocated */

void obj_temp(void *ptr);

/* Return true if object is a heap object, false if stack */

int obj_isperm(const void *ptr);

/* Reference length part of object */
#define obj_len(a) (obj_base(a)->len)

/* Reference malloc size part of object */
#define obj_size(a) (obj_base(a)->size)

/** Variable strings **/

/* Create a dynamic string with enough space for a string of indicated length. The created
 * string will have a length of 0. */
char *vsmk(int prealloc);

/* Get length of string, NULL allowed for s */
int vslen(const char *s);

/* Make sure there's enough space in the array for 'len' elements.  Whenever
 * vsensure reallocs the array, it allocates 25% more than the necessary
 * minimum space in anticipation of future expansion.  If 'vary' is 0,
 * it creates a new array. */
char *vsensure(char *s, int len);

/* Truncate array to indicated size.  This expands the string with blank
 * elements if necessary and sets the vslen() of the array.  A new array is
 * created if 'vary' is 0. */

char *vstrunc(char *s, int len);

/* Copy array/len to string starting at offset pos. String is expanded if
 * necessary to fit the array.  If pos is beyond length of string, string is
 * expanded with blanks. */

char *vsncpy(char *s, int pos, const char *array, int len);

/* Write a block of len ch characters starting at offset pos.  If pos is
   beyond length of string, the string is expanded with blanks. */

char *vsfill(char *s, int pos, int ch, int len);

/* Copy an array onto a string.  Same as: vstrunc(s,0), vscat(s,array). */

char *vscpy(char *s, char *array, int len);
char *vscpyz(char *s, const char *z);

/* Concatenate an array of characters to a string. */

char *vscat(char *s, char *array, int len);
char *vscatz(char *s, const char *z);

char *vsdup(const char *s);
char *vsdupz(const char *z);

/* Append a single character to a string */

char *_vsadd(char *s, char c);

#define vsadd(s, c) ( (s) && (obj_len(s) + 1) < obj_size(s) ? ((s)[obj_len(s)++] = c, (s)[obj_len(s)] = 0, (s)) : _vsadd((s), (c)) )

/* Assign a single character to a position in a string */

char *_vsset(char *vary, int pos, char el);

#define vsset(v, p, el)  \
 (!(v) || (p) > obj_len(v) || (p) >= obj_size(v) ?  \
  _vsset((v), (p), (el)) \
 : \
  ((p) == obj_len(v) ? \
   ((v)[(p) + 1] = 0, obj_len(v) = (p) + 1, (v)[p] = (el), (v)) \
  : \
   ((v)[p] = (el), (v)) \
  ) \
 )

/* Get line of input */

char *vsgets(char **s, FILE *f);

char **isfree(char **s);

int vsscan (char *a, int alen, char *b, int blen);

int vsspan (char *a, int alen, char *b, int blen);

int vsbsearch (char *ary, int len, char el);

/* Printf to variable string */

char *vsfmt(char *vary, int pos, const char *format, ...);

/** Variable arrays **/

char **vamk(int len);

void varm(char **vary);

char **vaensure(char **vary, int len);

char **vazap(char **vary, int pos, int n);

char **vatrunc(char **vary, int len);

char **vaadd(char **vary, char *s);

void vasort(char **vary, int len);
void vaisort(char **vary, int len);

void vadel(char **a, ptrdiff_t ofst, ptrdiff_t len);

void vauniq(char **a);

char **vawords(char **vary, char *s, int len, char *sep, int seplen);

#define sz(s) (s), zlen(s)
#define sv(s) (s), vslen(s)
#define sc(s) (char *)(s), (sizeof(s)-1)
#define av(a) (a), valen(a)

int valen(char **a);

void vaperm(char **a);
