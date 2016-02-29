/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

/* If we didn't find a CODESET in locale.h/langinfo.h, forget about setlocale */
#ifndef CODESET
#undef HAVE_SETLOCALE
#endif

/* UTF-8 Encoder
 *
 * c is unicode character.
 * buf is 7 byte buffer- utf-8 coded character is written to this followed by a 0 termination.
 * returns length (not including terminator).
 */

ptrdiff_t utf8_encode(char *buf,int c)
{
	if (c < 0x80) {
		buf[0] = TO_CHAR_OK(c);
		buf[1] = 0;
		return 1;
	} else if(c < 0x800) {
		buf[0] = TO_CHAR_OK((0xc0|(c>>6)));
		buf[1] = TO_CHAR_OK((0x80|(c&0x3F)));
		buf[2] = 0;
		return 2;
	} else if(c < 0x10000) {
		buf[0] = TO_CHAR_OK((0xe0|(c>>12)));
		buf[1] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[3] = 0;
		return 3;
	} else if(c < 0x200000) {
		buf[0] = TO_CHAR_OK((0xf0|(c>>18)));
		buf[1] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[4] = 0;
		return 4;
	} else if(c < 0x4000000) {
		buf[0] = TO_CHAR_OK((0xf8|(c>>24)));
		buf[1] = TO_CHAR_OK((0x80|((c>>18)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[4] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[5] = 0;
		return 5;
	} else {
		buf[0] = TO_CHAR_OK((0xfC|(c>>30)));
		buf[1] = TO_CHAR_OK((0x80|((c>>24)&0x3f)));
		buf[2] = TO_CHAR_OK((0x80|((c>>18)&0x3f)));
		buf[3] = TO_CHAR_OK((0x80|((c>>12)&0x3f)));
		buf[4] = TO_CHAR_OK((0x80|((c>>6)&0x3f)));
		buf[5] = TO_CHAR_OK((0x80|((c)&0x3f)));
		buf[6] = 0;
		return 6;
	}
}

/* UTF-8 Decoder
 *
 * Returns 0 - 7FFFFFFF: decoded character
 *                   -1: byte accepted, no character decoded yet.
 *                   -2: incomplete byte sequence
 *                   -3: no byte sequence started, but character is between 128 - 191, 254 or 255
 */

int utf8_decode(struct utf8_sm *utf8_sm,char c)
{
	if (utf8_sm->state) {
		if ((c&0xC0)==0x80) {
			utf8_sm->buf[utf8_sm->ptr++] = c;
			--utf8_sm->state;
			utf8_sm->accu = ((utf8_sm->accu<<6)|(c&0x3F));
			if(!utf8_sm->state)
				return utf8_sm->accu;
		} else {
			utf8_sm->state = 0;
			return UTF8_INCOMPLETE;
		}
	} else if ((c&0x80)==0x00) {
		/* 0 - 127 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 0;
		return c;
	} else if ((c&0xE0)==0xC0) {
		/* 192 - 223 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 1;
		utf8_sm->accu = (c&0x1F);
	} else if ((c&0xF0)==0xE0) {
		/* 224 - 239 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 2;
		utf8_sm->accu = (c&0x0F);
	} else if ((c&0xF8)==0xF0) {
		/* 240 - 247 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 3;
		utf8_sm->accu = (c&0x07);
	} else if ((c&0xFC)==0xF8) {
		/* 248 - 251 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 4;
		utf8_sm->accu = (c&0x03);
	} else if ((c&0xFE)==0xFC) {
		/* 252 - 253 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 5;
		utf8_sm->accu = (c&0x01);
	} else {
		/* 128 - 191, 254, 255 */
		utf8_sm->ptr = 0;
		utf8_sm->state = 0;
		return UTF8_BAD;
	}
	return UTF8_ACCEPTED;
}

/* Initialize state machine */

void utf8_init(struct utf8_sm *utf8_sm)
{
	utf8_sm->ptr = 0;
	utf8_sm->state = 0;
}

/* Decode first utf-8 sequence in a string */

int utf8_decode_string(const char *s)
{
	struct utf8_sm sm;
	int c;
	utf8_init(&sm);
	do
		c = utf8_decode(&sm, *s++);
		while (c == UTF8_ACCEPTED);
	return c;
}

/* Decode and advance
 *
 * Returns: 0 - 7FFFFFFF: decoded character
 *  UTF8_INCOMPLETE: incomplete sequence
 *  UTF8_BAD: bad start of sequence found.
 *
 * p/plen are always advanced in such a way that repeated called to utf8_decode_fwrd do not cause
 * infinite loops.
 *
 * Pass NULL in plen for zero-terminated strings
 */

int utf8_decode_fwrd(const char **p,ptrdiff_t *plen)
{
	struct utf8_sm sm;
	const char *s = *p;
	ptrdiff_t len;
	int c = UTF8_INCOMPLETE; /* Return this on no more input. */
	if (plen)
		len = *plen;
	else
		len = -1;

	utf8_init(&sm);

	while (plen ? (len != 0) : (*s != 0)) {
		c = utf8_decode(&sm, *s);
		if (c >= 0) {
			/* We've got a character */
			--len;
			++s;
			break;
		} else if (c == UTF8_INCOMPLETE) {
			/* Bad sequence detected.  Caller should feed rest of string in again. */
			break;
		} else if (c == UTF8_BAD) {
			/* Bad start of UTF-8 sequence.  We need to eat this char to avoid infinite loops. */
			--len;
			++s;
			/* But we should tell the caller that something bad was found. */
			break;
		} else {
			/* If c is -1, utf8_decode accepted the character, so we should get the next one. */
			--len;
			++s;
		}
	}

	if (plen)
		*plen = len;
	*p = s;

	return c;
}

