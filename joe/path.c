/* 
 *	Directory and path functions
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#ifdef JOEWIN
// Needs to come before joe_redefines.h
#include <direct.h>
#endif

#include "types.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_PATHS_H
#  include <paths.h>	/* for _PATH_TMP */
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


#ifndef JOEWIN
#  ifdef HAVE_DIRENT_H
#    include <dirent.h>
#    define NAMLEN(dirent) zlen((dirent)->d_name)
#  else
#    ifdef HAVE_SYS_DIRENT_H
#      include <sys/dirent.h>
#      define NAMLEN(dirent) zlen((dirent)->d_name)
#    else
#      define direct dirent
#      define NAMLEN(dirent) (dirent)->d_namlen
#      ifdef HAVE_SYS_NDIR_H
#        include <sys/ndir.h>
#      else
#        ifdef HAVE_SYS_DIR_H
#          include <sys/dir.h>
#        else
#          ifdef HAVE_NDIR_H
#            include <ndir.h>
#          else
#            ifndef __MSDOS__
#              include "dir.c"
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif

#if defined(__MSDOS__) || defined(JOEWIN)	/* paths in MS-DOS can include a drive letter followed by semicolon */
#define	do_if_drive_letter(path, command) do { \
						if ((path)[0] && (path)[1] == ':') { \
							command; \
						} \
					} while(0)
#else
#define do_if_drive_letter(path, command)	do { } while(0)
#endif
#define skip_drive_letter(path)	do_if_drive_letter((path), (path) += 2)

#ifndef		_PATH_TMP
#  if defined(__MSDOS__) || defined(JOEWIN)
#    define	_PATH_TMP	""
#  else
#    define	_PATH_TMP	"/tmp/"
#  endif
#endif

#ifndef PATH_MAX
#warning What should we include to have PATH_MAX defined?
#define PATH_MAX	4096
#endif

/********************************************************************/
char *joesep(char *path)
{
#if defined(__MSDOS__) || defined(JOEWIN)
	int x;

	for (x = 0; path[x]; ++x)
		if (path[x] == '\\')
			path[x] = '/';
#endif
	return path;
}
/********************************************************************/
char *namprt(const char *path)
{
	const char *z;

	skip_drive_letter(path);
	z = path + zlen(path);
	while ((z != path) && !ISDIRSEP(z[-1]))
		--z;
	return vsncpy(NULL, 0, sz(z));
}
/********************************************************************/
char *namepart(char *tmp, ptrdiff_t tmpsiz, const char *path)
{
	const char *z;

	skip_drive_letter(path);
	z = path + zlen(path);
	while ((z != path) && !ISDIRSEP(z[-1]))
		--z;
	return zlcpy(tmp, tmpsiz, z);
}
/********************************************************************/
char *dirprt(const char *path)
{
	const char *b = path;
	const char *z = path + zlen(path);

	skip_drive_letter(b);
	while ((z != b) && !ISDIRSEP(z[-1]))
		--z;
	return vsncpy(NULL, 0, path, z - path);
}
/********************************************************************/
char *begprt(const char *path)
{
	const char *z = path + zlen(path);
	int drv = 0;

	do_if_drive_letter(path, drv = 2);
	while ((z != path + drv) && ISDIRSEP(z[-1]))
		--z;
	if (z == path + drv)
		return vsncpy(NULL, 0, sz(path));
	else {
		while ((z != path + drv) && !ISDIRSEP(z[-1]))
			--z;
		return vsncpy(NULL, 0, path, z - path);
	}
}
/********************************************************************/
char *endprt(const char *path)
{
	const char *z = path + zlen(path);
	int drv = 0;

	do_if_drive_letter(path, drv = 2);
	while ((z != path + drv) && ISDIRSEP(z[-1]))
		--z;
		if (z == path + drv)
		return vsncpy(NULL, 0, sc(""));
	else {
		while (z != path + drv && !ISDIRSEP(z[-1]))
			--z;
		return vsncpy(NULL, 0, sz(z));
	}
}
/********************************************************************/
int mkpath(char *path)
{
	char *s;

	if (ISDIRSEP(path[0])) {
		if (chddir("/"))
			return 1;
		s = path;
		goto in;
	}

	while (path[0]) {
		char c;

		for (s = path; (*s) && !ISDIRSEP(*s); s++) ;
		c = *s;
		*s = 0;
		if (chddir(path)) {
			if (mkdir(path, 0777))
			return 1;
			if (chddir(path))
				return 1;
		}
		*s = c;
	      in:
		while (ISDIRSEP(*s))
			++s;
		path = s;
	}
	return 0;
}
/********************************************************************/
/* Create a temporary file */
/********************************************************************/
char *mktmp(const char *where)
{
#ifndef HAVE_MKSTEMP
	static unsigned seq = 0;
#endif
	char *name;
	int fd;
	ptrdiff_t namesize;

	if (!where)
		where = getenv("TEMP");
	if (!where)
		where = _PATH_TMP;

	namesize = zlen(where) + 16;
	name = vsmk(namesize);	/* [G.Ghibo'] we need to use vsmk() and not malloc() as
				   area returned by mktmp() is destroyed later with
				   vsrm(); */
#ifdef HAVE_MKSTEMP
	name = vsfmt(name, 0, "%s/joe.tmp.XXXXXX", where);
	if((fd = mkstemp(name)) == -1)
		return NULL;	/* FIXME: vflsh() and vflshf() */
				/* expect mktmp() always succeed!!! */

	fchmod(fd, 0600);       /* Linux glibc 2.0 mkstemp() creates it with */
				/* 0666 mode --> change it to 0600, so nobody */
				/* else sees content of temporary file */
	close(fd);

#else
      loop:
	seq = (seq + 1) % 1000;
	name = vsfmt(name, 0, "%s/joe.tmp.%03u%03u", where, seq, (unsigned) time(NULL) % 1000);
	if ((fd = open(name, O_RDONLY)) != -1) {
		close(fd);
		goto loop;	/* FIXME: possible endless loop --> DoS attack */
	}
	if ((fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
		return NULL;	/* FIXME: see above */
	else
		close(fd);
#endif
	obj_perm(name);
	return name;
}
/********************************************************************/
int rmatch(const char *a, const char *b, int fs)
{
	int flag, inv, c;

	for (;;)
		switch (*a) {
		case '*':
			++a;
			do {
				if (rmatch(a, b, fs))
					return 1;
			} while (*b++);
			return 0;
		case '[':
			++a;
			flag = 0;
			if (*a == '^') {
				++a;
				inv = 1;
			} else
				inv = 0;
			if (*a == ']')
				if (*b == *a++)
					flag = 1;
			while (*a && (c = *(const unsigned char *)a++) != ']')
				if ((c == '-') && (a[-2] != '[') && (*a)) {
					if ((fs && (MATCHCANON(*(const unsigned char *)b) >= MATCHCANON(((const unsigned char *)a)[-2])) && (MATCHCANON(*(const unsigned char *)b) <= MATCHCANON(*(const unsigned char *)a))) ||
						(!fs && (*(const unsigned char *)b >= ((const unsigned char *)a)[-2]) && (*(const unsigned char *)b <= *(const unsigned char *)a)))
						flag = 1;
				} else if ((fs && MATCHCANON(*(const unsigned char *)b) == MATCHCANON(c)) || (!fs && *(const unsigned char *)b == c))
					flag = 1;
			if ((!flag && !inv) || (flag && inv) || (!*b))
				return 0;
			++b;
			break;
		case '?':
			++a;
			if (!*b)
				return 0;
			++b;
			break;
		case 0:
			if (!*b)
				return 1;
			else
				return 0;
		default:
			if ((fs && MATCHCANON(*a++) != MATCHCANON(*b++)) || (!fs && *a++ != *b++))
				return 0;
		}
}
/********************************************************************/
int isreg(const char *s)
{
	int x;

	for (x = 0; s[x]; ++x)
		if ((s[x] == '*') || (s[x] == '?') || (s[x] == '['))
			return 1;
	return 0;
}
/********************************************************************/
#ifdef __MSDOS__
#include <dos.h>
#include <dir.h>

struct direct {
	char d_name[16];
} direc;
int dirstate = 0;
struct ffblk ffblk;
char *dirpath = NULL;

void *opendir(char *path)
{
	dirstate = 0;
	return &direc;
}

void closedir()
{
}

struct direct *readdir()
{
	int x;

	if (dirstate) {
		if (findnext(&ffblk))
			return NULL;
	} else {
		if (findfirst("*.*", &ffblk, FA_DIREC))
			return NULL;
		dirstate = 1;
	}

	zlcpy(direc.d_name, SIZEOF(direc.d_name), ffblk.ff_name);
	for (x = 0; direc.d_name[x]; ++x)
		direc.d_name[x] = tolower(direc.d_name[x]);
	return &direc;
}
#endif
/********************************************************************/
char **rexpnd(const char *word)
{
	DIR *dir;
	char **lst = NULL;

	DIR *de;
	dir = opendir(".");
	if (dir) {
		while ((de = readdir(dir)) != NULL)
			if (zcmp(".", de->d_name))
				if (rmatch(word, de->d_name, 1))
					lst = vaadd(lst, vsncpy(NULL, 0, sz(de->d_name)));
		closedir(dir);
	}
	return lst;
}

char **rexpnd_cmd_cd(const char *word)
{
	DIR *dir;
	char **lst = NULL;

	struct dirent *de;
	dir = opendir(".");
	if (dir) {
		while ((de = readdir(dir)) != NULL)
			if (zcmp(".", de->d_name))
				if (rmatch(word, de->d_name, 1) && !access(de->d_name, X_OK))
					lst = vaadd(lst, vsncpy(NULL, 0, sz(de->d_name)));
		closedir(dir);
	}
	return lst;
}

char **rexpnd_cmd_path(const char *word)
{
	char *raw_path = getenv("PATH");
	char **lst = NULL;
	DIR *de;
	char **path;
	ptrdiff_t x;
	if (raw_path) {
#ifndef JOEWIN
		path = vawords(NULL, sz(raw_path), sc(":"));
#else
		path = vawords(NULL, sz(raw_path), sc(";"));
#endif
		for (x = 0; path[x]; ++x) {
			DIR *dir;
			if (!chpwd(path[x])) {
				dir = opendir(".");
				if (dir) {
					while ((de = readdir(dir)) != NULL) {
						struct stat buf[1];
						if (rmatch(word, de->d_name, 1) && !stat(de->d_name, buf) && S_ISREG(buf->st_mode) && !access(de->d_name, X_OK))
							lst = vaadd(lst, vsncpy(NULL, 0, sz(de->d_name)));
					}
					closedir(dir);
				}
			}
		}
		varm(path);
	}
	return lst;
}

/********************************************************************/
char **rexpnd_users(const char *word)
{
	char **lst = NULL;

#ifndef JOEWIN
	struct passwd *pw;

	while((pw=getpwent()))
		if (rmatch(word+1, pw->pw_name, 1)) {
			char *t = vsncpy(NULL,0,sc("~"));
			lst = vaadd(lst, vsncpy(sv(t),sz(pw->pw_name)));
			}
	endpwent();
#endif

	return lst;
}
/********************************************************************/
int chpwd(const char *path)
{
#if defined(__MSDOS__) || defined(JOEWIN)
	char buf[1024];
	int x;

	if (!path)
		return 0;
	if ((path[0]) && (path[1] == ':')) {
		if (_chdrive(path[0] & 0x1F))
			return -1;
		path += 2;
	}
	if (!path[0])
		return 0;
	zlcpy(buf, SIZEOF(buf), path);
	x = zlen(buf);
	while (x > 1) {
		--x;
		if (ISDIRSEP(buf[x]))
			buf[x] = 0;
		else
			break;
	}
	return chdir(buf);
#else
	if ((!path) || (!path[0]))
		return 0;
	return chdir(path);
#endif
}

/* The pwd function */
char *pwd(void)
{
	static char buf[PATH_MAX];
	char	*ret;

#ifdef HAVE_GETCWD
	ret = getcwd(buf, PATH_MAX - 1);
#else
	ret = getwd(buf);
#endif
	buf[PATH_MAX - 1] = '\0';

	return ret;
}

/* Simplify prefix by using ~ */
/* Expects s to have trailing / */

char *simplify_prefix(const char *s)
{
#ifdef JOEWIN
	return vsncpy(NULL,0,sz(s));
#else
	const char *t = getenv("HOME");
	char *n;

#ifdef junk
	char *org = pwd();
	/* Normalize home */
	if (t && !chpwd(t)) {
		t = pwd();
	} else {
		t = 0;
	}
	chpwd(org);
#endif

	/* If current directory is prefixed with home directory, use ~... */
	if (t && !zncmp(s,t,zlen(t)) && (!s[zlen(t)] || s[zlen(t)]=='/')) {
		n = vsncpy(NULL,0,sc("~/"));
		/* If anything more than just the home directory, add it */
		if (s[zlen(t)]) {
			n = vsncpy(sv(n),s+zlen(t)+1,zlen(s+zlen(t)+1));
		}
	} else {
		n = vsncpy(NULL,0,sz(s));
	}
	return n;
#endif
}

#ifndef JOEWIN

char *dequotevs(char *s)
{
	ptrdiff_t x;
	char *d = vsensure(NULL, vslen(s));
	for (x = 0; x != vslen(s); ++x)
		if (s[x] == '\\') {
			/* Just skip it */
		} else
			d = vsadd(d, s[x]);
	return d;
}

#else

char *dequotevs(char *s)
{
	ptrdiff_t x;
	char *d = vsensure(NULL, vslen(s));
	for (x = 0; x != vslen(s); ++x)
		if (s[x] == '"') {
			/* Just skip it */
		} else
			d = vsadd(d, s[x]);
	return d;
}

#endif
