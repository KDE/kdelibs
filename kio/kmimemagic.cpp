/* This file is part of the KDE libraries
   Copyright (C) 2000 Fritz Elfert <fritz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kmimemagic.h"
#include <kdebug.h>
#include <kapp.h>
#include <qfile.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kstaticdeleter.h>
#include <assert.h>

KMimeMagic* KMimeMagic::s_pSelf = 0L;
KStaticDeleter<KMimeMagic> kmimemagicsd;

KMimeMagic* KMimeMagic::self()
{
  if( !s_pSelf )
    initStatic();
  return s_pSelf;
}

void KMimeMagic::initStatic()
{
  // Magic file detection init
  QString mimefile = locate( "mime", "magic" );
  s_pSelf = kmimemagicsd.setObject( new KMimeMagic( mimefile ) );
  s_pSelf->setFollowLinks( TRUE );
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <utime.h>
#include <stdarg.h>
#include <qregexp.h>
#include <qstring.h>

/*
 * data structures and related constants
 */
#define MIME_MAGIC_DEBUG_TABLE 0
//#define DEBUG_MIMEMAGIC 1

#define DECLINED 999
#define ERROR    998
#define OK         0

/*
 * Buitltin Mime types
 */
#define MIME_BINARY_UNKNOWN    "application/octet-stream"
#define MIME_BINARY_UNREADABLE "application/x-unreadable"
#define MIME_BINARY_ZEROSIZE   "application/x-zerosize"
#define MIME_TEXT_UNKNOWN      "text/plain"
#define MIME_TEXT_PLAIN        "text/plain"
#define MIME_INODE_DIR         "inode/directory"
#define MIME_INODE_CDEV        "inode/chardevice"
#define MIME_INODE_BDEV        "inode/blockdevice"
#define MIME_INODE_FIFO        "inode/fifo"
#define MIME_INODE_LINK        "inode/link"
#define MIME_INODE_SOCK        "inode/socket"
// Following should go in magic-file - Fritz
#define MIME_APPL_TROFF        "application/x-troff"
#define MIME_APPL_TAR          "application/x-tar"
#define MIME_TEXT_FORTRAN      "text/x-fortran"

#define MAXMIMESTRING        256

#define HOWMANY 1024            /* big enough to recognize most WWW files */
#define MAXDESC   50            /* max leng of text description */
#define MAXstring 64            /* max leng of "string" types */

typedef union VALUETYPE {
	unsigned char b;
	unsigned short h;
	unsigned long l;
	char s[MAXstring];
	unsigned char hs[2];	/* 2 bytes of a fixed-endian "short" */
	unsigned char hl[4];	/* 2 bytes of a fixed-endian "long" */
} VALUETYPE;

struct magic {
	struct magic *next;     /* link to next entry */
	int lineno;             /* line number from magic file */

	short flag;
#define INDIR    1              /* if '>(...)' appears,  */
#define UNSIGNED 2              /* comparison is unsigned */
	short cont_level;       /* level of ">" */
	struct {
		char type;      /* byte short long */
		long offset;    /* offset from indirection */
	} in;
	long offset;            /* offset to magic number */
	unsigned char reln;     /* relation (0=eq, '>'=gt, etc) */
	char type;              /* int, short, long or string. */
	char vallen;            /* length of string value, if any */
#define BYTE       1
#define SHORT      2
#define LONG       4
#define STRING     5
#define DATE       6
#define BESHORT    7
#define BELONG     8
#define BEDATE     9
#define LESHORT   10
#define LELONG    11
#define LEDATE    12
	VALUETYPE value;        /* either number or string */
	unsigned long mask;     /* mask before comparison with value */
	char nospflag;          /* supress space character */

	/* NOTE: this string is suspected of overrunning - find it! */
	char desc[MAXDESC];     /* description */
};

/*
 * data structures for tar file recognition
 * --------------------------------------------------------------------------
 * Header file for public domain tar (tape archive) program.
 *
 * @(#)tar.h 1.20 86/10/29    Public Domain. Created 25 August 1985 by John
 * Gilmore, ihnp4!hoptoad!gnu.
 *
 * Header block on tape.
 *
 * I'm going to use traditional DP naming conventions here. A "block" is a big
 * chunk of stuff that we do I/O on. A "record" is a piece of info that we
 * care about. Typically many "record"s fit into a "block".
 */
#define RECORDSIZE    512
#define NAMSIZ    100
#define TUNMLEN    32
#define TGNMLEN    32

union record {
	char charptr[RECORDSIZE];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
		char magic[8];
		char uname[TUNMLEN];
		char gname[TGNMLEN];
		char devmajor[8];
		char devminor[8];
	} header;
};

/* The magic field is filled with this if uname and gname are valid. */
#define    TMAGIC        "ustar  "	/* 7 chars and a null */

/*
 * file-function prototypes
 */
static int is_tar(unsigned char *, int);
static unsigned long signextend(struct magic *, unsigned long);
static int getvalue(struct magic *, char **);
static int hextoint(int);
static char *getstr(char *, char *, int, int *);
static int mget(union VALUETYPE *, unsigned char *, struct magic *, int);
static int mcheck(union VALUETYPE *, struct magic *);
static int mconvert(union VALUETYPE *, struct magic *);
static long from_oct(int, char *);

/*
 * includes for ASCII substring recognition formerly "names.h" in file
 * command
 *
 * Original notes: names and types used by ascmagic in file(1).
 * These tokens are
 * here because they can appear anywhere in the first HOWMANY bytes, while
 * tokens in /etc/magic must appear at fixed offsets into the file. Don't
 * make HOWMANY too high unless you have a very fast CPU.
 */

/* these types are used calculate index to 'types': keep em in sync! */
/* HTML inserted in first because this is a web server module now */
/* ENG removed because stupid */
#define L_HTML   0x001          /* HTML */
#define L_C      0x002          /* first and foremost on UNIX */
#define L_MAKE   0x004          /* Makefiles */
#define L_PLI    0x008          /* PL/1 */
#define L_MACH   0x010          /* some kinda assembler */
#define L_PAS    0x020          /* Pascal */
#define L_JAVA   0x040          /* Java source */
#define L_CPP    0x080          /* C++ */
#define L_MAIL   0x100          /* Electronic mail */
#define L_NEWS   0x200          /* Usenet Netnews */
#define L_DIFF   0x400          /* Output of diff */

#define P_HTML   0          /* HTML */
#define P_C      1          /* first and foremost on UNIX */
#define P_MAKE   2          /* Makefiles */
#define P_PLI    3          /* PL/1 */
#define P_MACH   4          /* some kinda assembler */
#define P_PAS    5          /* Pascal */
#define P_JAVA   6          /* Java source */
#define P_CPP    7          /* C++ */
#define P_MAIL   8          /* Electronic mail */
#define P_NEWS   9          /* Usenet Netnews */
#define P_DIFF  10          /* Output of diff */

typedef struct asc_type {
	const char *type;
	int  kwords;
	double  weight;
} asc_type;

static asc_type types[] = {
	{ "text/html",          6, 2 }, // 10 items but 6 different words only
	{ "text/x-c",           9, 1.3 },
	{ "text/x-makefile",    4, 1.9 },
	{ "text/x-pli",         1, 3 },
	{ "text/x-assembler",   6, 2.1 },
	{ "text/x-pascal",      1, 1 },
	{ "text/x-java",       14, 1 },
	{ "text/x-c++",        14, 1 },
	{ "message/rfc822",     4, 1.9 },
	{ "message/news",       3, 2 },
        { "text/x-diff",        4, 2 }
};

#define NTYPES (sizeof(types)/sizeof(asc_type))

static struct names {
	const char *name;
	short type;
} names[] = {
	{
		"<html>", L_HTML
	},
	{
		"<HTML>", L_HTML
	},
	{
		"<head>", L_HTML
	},
	{
		"<HEAD>", L_HTML
	},
	{
		"<title>", L_HTML
	},
	{
		"<TITLE>", L_HTML
	},
	{
		"<h1>", L_HTML
	},
	{
		"<H1>", L_HTML
	},
	{
		"<a", L_HTML
	},
	{
		"<A", L_HTML
	},
	{
		"<img", L_HTML
	},
	{
		"<IMG", L_HTML
	},
	{
		"<!--", L_HTML
	},
	{
		"<!doctype", L_HTML
	},
	{
		"<!DOCTYPE", L_HTML
	},
	{
		"<div", L_HTML
	},
	{
		"<DIV", L_HTML
	},
	{
		"/*", L_C|L_CPP|L_JAVA
	},                      /* must precede "The", "the", etc. */
	{
		"//", L_CPP|L_JAVA
	},                      /* must precede "The", "the", etc. */
	{
		"#include", L_C|L_CPP
	},
	{
		"char", L_C|L_CPP|L_JAVA
	},
	{
		"double", L_C|L_CPP|L_JAVA
	},
	{
		"extern", L_C|L_CPP
	},
	{
		"float", L_C|L_CPP|L_JAVA
	},
	{
		"real", L_C|L_CPP|L_JAVA
	},
	{
		"struct", L_C|L_CPP
	},
	{
		"union", L_C|L_CPP
	},
	{
		"implements", L_JAVA
	},
	{
		"super", L_JAVA
	},
	{
		"import", L_JAVA
	},
	{
		"class", L_CPP|L_JAVA
	},
	{
		"public", L_CPP|L_JAVA
	},
	{
		"private", L_CPP|L_JAVA
	},
	{
		"CFLAGS", L_MAKE
	},
	{
		"LDFLAGS", L_MAKE
	},
	{
		"all:", L_MAKE
	},
	{
		".PRECIOUS", L_MAKE
	},
	/*
	 * Too many files of text have these words in them.  Find another way
	 * to recognize Fortrash.
	 */
	{
		".ascii", L_MACH
	},
	{
		".asciiz", L_MACH
	},
	{
		".byte", L_MACH
	},
	{
		".even", L_MACH
	},
	{
		".globl", L_MACH
	},
	{
		"clr", L_MACH
	},
	{
		"(input", L_PAS
	},
	{
		"dcl", L_PLI
	},
	{
		"Received:", L_MAIL
	},
	{
		">From", L_MAIL
	},
	{
		"Return-Path:", L_MAIL
	},
	{
		"Cc:", L_MAIL
	},
	{
		"Newsgroups:", L_NEWS
	},
	{
		"Path:", L_NEWS
	},
	{
		"Organization:", L_NEWS
	},
	{
		"---", L_DIFF
	},
	{
		"+++", L_DIFF
	},
	{
		"***", L_DIFF
	},
	{
		"@@", L_DIFF
	},
	{
		NULL, 0
	}
};

/**
 * Configuration for the utime() problem.
 * Here's the problem:
 * By looking into a file to determine its mimetype, we change its "last access"
 * time (atime) and this can have side effects, like files in /tmp never being
 * cleaned up because of that. So in temp directories, we restore the atime.
 * Since this changes the ctime (last change of attributes), we don't do that
 * anywhere else, because that breaks archiving programs, that check the ctime.
 * Hence this class, to configure the directories where the atime should be restored.
 */
class KMimeMagicUtimeConf
{
public:
    KMimeMagicUtimeConf()
    {
        tmpDirs << QString::fromLatin1("/tmp"); // default value

        // The trick is that we also don't want the user to override globally set
        // directories. So we have to misuse KStandardDirs :}
        QStringList confDirs = KGlobal::dirs()->resourceDirs( "config" );
        if ( !confDirs.isEmpty() )
        {
            QString globalConf = confDirs.last() + "kmimemagicrc";
            if ( QFile::exists( globalConf ) )
            {
                KSimpleConfig cfg( globalConf );
                cfg.setGroup( "Settings" );
                tmpDirs = cfg.readListEntry( "atimeDirs" );
            }
            if ( confDirs.count() > 1 )
            {
                QString localConf = confDirs.first() + "kmimemagicrc";
                if ( QFile::exists( localConf ) )
                {
                    KSimpleConfig cfg( localConf );
                    cfg.setGroup( "Settings" );
                    tmpDirs += cfg.readListEntry( "atimeDirs" );
                }
            }
            for ( QStringList::Iterator it = tmpDirs.begin() ; it != tmpDirs.end() ; ++it )
            {
                QString dir = *it;
                if ( !dir.isEmpty() && dir[ dir.length()-1 ] != '/' )
                    (*it) += '/';
            }
        }
#if 0
        // debug code
        for ( QStringList::Iterator it = tmpDirs.begin() ; it != tmpDirs.end() ; ++it )
            kdDebug() << " atimeDir: " << *it << endl;
#endif
    }

    bool restoreAccessTime( const QString & file ) const
    {
        QString dir = file.left( file.findRev( '/' ) );
        bool res = tmpDirs.contains( dir );
        //kdDebug(7018) << "restoreAccessTime " << file << " dir=" << dir << " result=" << res << endl;
        return res;
    }
    QStringList tmpDirs;
};

/* current config */
struct config_rec {
	QString magicfile;      /* where magic be found      */
	struct magic *magic,    /* head of magic config list */
	*last;
        KMimeMagicUtimeConf * utimeConf;
};

#if (MIME_MAGIC_DEBUG_TABLE > 1)
static void
test_table()
{
	struct magic *m;
	struct magic *prevm = NULL;

	// debug("test_table : started", );
	for (m = conf->magic; m; m = m->next) {
		if (isprint((((unsigned long) m) >> 24) & 255) &&
		    isprint((((unsigned long) m) >> 16) & 255) &&
		    isprint((((unsigned long) m) >> 8) & 255) &&
		    isprint(((unsigned long) m) & 255)) {
		    //debug("test_table: POINTER CLOBBERED! "
		    //"m=\"%c%c%c%c\" line=%d",
			      (((unsigned long) m) >> 24) & 255,
			      (((unsigned long) m) >> 16) & 255,
			      (((unsigned long) m) >> 8) & 255,
			      ((unsigned long) m) & 255,
			      prevm ? prevm->lineno : -1);
			break;
		}
		prevm = m;
	}
}
#endif

#define    EATAB {while (isascii((unsigned char) *l) && \
	      isspace((unsigned char) *l))  ++l;}

int KMimeMagic::parse_line(char *line, int *rule, int lineno)
{
	int ws_offset;

	/* delete newline */
	if (line[0]) {
		line[strlen(line) - 1] = '\0';
	}
	/* skip leading whitespace */
	ws_offset = 0;
	while (line[ws_offset] && isspace(line[ws_offset])) {
		ws_offset++;
	}

	/* skip blank lines */
	if (line[ws_offset] == 0) {
		return 0;
	}
	/* comment, do not parse */
	if (line[ws_offset] == '#')
		return 0;

	/* if we get here, we're going to use it so count it */
	(*rule)++;

	/* parse it */
	return (parse(line + ws_offset, lineno) != 0);
}

/*
 * apprentice - load configuration from the magic file.
 */
int KMimeMagic::apprentice()
{
	FILE *f;
	char line[BUFSIZ + 1];
	int errs = 0;
	int lineno;
	int rule = 0;
	QCString fname;

	if (conf->magicfile.isEmpty())
		return -1;
	fname = QFile::encodeName(conf->magicfile);
	f = fopen(fname, "r");
	if (f == NULL) {
		kdError(7018) << "can't read magic file " << fname.data() << ": " << strerror(errno) << endl;
		return -1;
	}

	/* parse it */
	for (lineno = 1; fgets(line, BUFSIZ, f) != NULL; lineno++)
		if (parse_line(line, &rule, lineno))
			errs++;

	fclose(f);

	//kdDebug(7018) << "apprentice: conf=" << conf << " file=" << conf->magicfile.local8Bit().data() << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	//kdDebug(7018) << "apprentice: read " << lineno << " lines, " << rule << " rules, " << errs << " errors" << endl;

#if (MIME_MAGIC_DEBUG_TABLE > 1)
	test_table();
#endif

	return (errs ? -1 : 0);
}

int KMimeMagic::buff_apprentice(char *buff)
{
	char line[BUFSIZ + 2];
	int errs = 0;
	int lineno = 1;
	char *start = buff;
	char *end;
	int count = 0;
	int rule = 0;
	int len = strlen(buff) + 1;

	/* parse it */
	do {
		count = (len > BUFSIZ-1)?BUFSIZ-1:len;
		strncpy(line, start, count);
		line[count] = '\0';
		if ((end = strchr(line, '\n'))) {
			*(++end) = '\0';
			count = strlen(line);
		} else
		  strcat(line, "\n");
		start += count;
		len -= count;
		if (parse_line(line, &rule, lineno))
			errs++;
		lineno++;
	} while (len > 0);

	//kdDebug(7018) << "buff_apprentice: conf=" << conf << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	//kdDebug(7018) << "buff_apprentice: read " << lineno << " lines, " << rule << " rules, " << errs << " errors" << endl;

#if ( MIME_MAGIC_DEBUG_TABLE > 1 )
	test_table();
#endif

	return (errs ? -1 : 0);
}

/*
 * extend the sign bit if the comparison is to be signed
 */
static unsigned long
signextend(struct magic *m, unsigned long v)
{
	if (!(m->flag & UNSIGNED))
		switch (m->type) {
				/*
				 * Do not remove the casts below.  They are vital.
				 * When later compared with the data, the sign
				 * extension must have happened.
				 */
			case BYTE:
				v = (char) v;
				break;
			case SHORT:
			case BESHORT:
			case LESHORT:
				v = (short) v;
				break;
			case DATE:
			case BEDATE:
			case LEDATE:
			case LONG:
			case BELONG:
			case LELONG:
				v = (long) v;
				break;
			case STRING:
				break;
			default:
				kdError(7018) << "" << "signextend" << ": can't happen: m->type=" << m->type << endl;
				return ERROR;
		}
	return v;
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
int KMimeMagic::parse(char *l, int lineno)
{
	int i = 0;
	struct magic *m;
	char *t,
	*s;
	/* allocate magic structure entry */
	if ((m = (struct magic *) calloc(1, sizeof(struct magic))) == NULL) {
		kdError(7018) << "parse: Out of memory." << endl;
		return -1;
	}
	/* append to linked list */
	m->next = NULL;
	if (!conf->magic || !conf->last) {
		conf->magic = conf->last = m;
	} else {
		conf->last->next = m;
		conf->last = m;
	}

	/* set values in magic structure */
	m->flag = 0;
	m->cont_level = 0;
	m->lineno = lineno;

	while (*l == '>') {
		++l;            /* step over */
		m->cont_level++;
	}

	if (m->cont_level != 0 && *l == '(') {
		++l;            /* step over */
		m->flag |= INDIR;
	}
	/* get offset, then skip over it */
	m->offset = (int) strtol(l, &t, 0);
	if (l == t) {
            kdError(7018) << "parse: offset " << l << " invalid" << endl;
	}
	l = t;

	if (m->flag & INDIR) {
		m->in.type = LONG;
		m->in.offset = 0;
		/*
		 * read [.lbs][+-]nnnnn)
		 */
		if (*l == '.') {
			switch (*++l) {
				case 'l':
					m->in.type = LONG;
					break;
				case 's':
					m->in.type = SHORT;
					break;
				case 'b':
					m->in.type = BYTE;
					break;
				default:
					kdError(7018) << "parse: indirect offset type " << *l << " invalid" << endl;
					break;
			}
			l++;
		}
		s = l;
		if (*l == '+' || *l == '-')
			l++;
		if (isdigit((unsigned char) *l)) {
			m->in.offset = strtol(l, &t, 0);
			if (*s == '-')
				m->in.offset = -m->in.offset;
		} else
			t = l;
		if (*t++ != ')') {
			kdError(7018) << "parse: missing ')' in indirect offset" << endl;
		}
		l = t;
	}
	while (isascii((unsigned char) *l) && isdigit((unsigned char) *l))
		++l;
	EATAB;

#define NBYTE       4
#define NSHORT      5
#define NLONG       4
#define NSTRING     6
#define NDATE       4
#define NBESHORT    7
#define NBELONG     6
#define NBEDATE     6
#define NLESHORT    7
#define NLELONG     6
#define NLEDATE     6

	if (*l == 'u') {
		++l;
		m->flag |= UNSIGNED;
	}
	/* get type, skip it */
	if (strncmp(l, "byte", NBYTE) == 0) {
		m->type = BYTE;
		l += NBYTE;
	} else if (strncmp(l, "short", NSHORT) == 0) {
		m->type = SHORT;
		l += NSHORT;
	} else if (strncmp(l, "long", NLONG) == 0) {
		m->type = LONG;
		l += NLONG;
	} else if (strncmp(l, "string", NSTRING) == 0) {
		m->type = STRING;
		l += NSTRING;
	} else if (strncmp(l, "date", NDATE) == 0) {
		m->type = DATE;
		l += NDATE;
	} else if (strncmp(l, "beshort", NBESHORT) == 0) {
		m->type = BESHORT;
		l += NBESHORT;
	} else if (strncmp(l, "belong", NBELONG) == 0) {
		m->type = BELONG;
		l += NBELONG;
	} else if (strncmp(l, "bedate", NBEDATE) == 0) {
		m->type = BEDATE;
		l += NBEDATE;
	} else if (strncmp(l, "leshort", NLESHORT) == 0) {
		m->type = LESHORT;
		l += NLESHORT;
	} else if (strncmp(l, "lelong", NLELONG) == 0) {
		m->type = LELONG;
		l += NLELONG;
	} else if (strncmp(l, "ledate", NLEDATE) == 0) {
		m->type = LEDATE;
		l += NLEDATE;
	} else {
		kdError(7018) << "parse: type " << l << " invalid" << endl;
		return -1;
	}
	/* New-style anding: "0 byte&0x80 =0x80 dynamically linked" */
	if (*l == '&') {
		++l;
		m->mask = signextend(m, strtol(l, &l, 0));
	} else
		m->mask = (unsigned long) ~0L;
	EATAB;

	switch (*l) {
		case '>':
		case '<':
			/* Old-style anding: "0 byte &0x80 dynamically linked" */
		case '&':
		case '^':
		case '=':
			m->reln = *l;
			++l;
			break;
		case '!':
			if (m->type != STRING) {
				m->reln = *l;
				++l;
				break;
			}
			/* FALL THROUGH */
		default:
			if (*l == 'x' && isascii((unsigned char) l[1]) &&
			    isspace((unsigned char) l[1])) {
				m->reln = *l;
				++l;
				goto GetDesc;	/* Bill The Cat */
			}
			m->reln = '=';
			break;
	}
	EATAB;

	if (getvalue(m, &l))
		return -1;
	/*
	 * now get last part - the description
	 */
      GetDesc:
	EATAB;
	if (l[0] == '\b') {
		++l;
		m->nospflag = 1;
	} else if ((l[0] == '\\') && (l[1] == 'b')) {
		++l;
		++l;
		m->nospflag = 1;
	} else
		m->nospflag = 0;
	while ((m->desc[i++] = *l++) != '\0' && i < MAXDESC)
		/* NULLBODY */ ;

	//kdDebug(7018) << "parse: line=" << lineno << " m=" << m << " next=" << m->next << " cont=" << m->cont_level << " desc=" << (m->desc ? m->desc : "NULL") << endl;
	return 0;
}

/*
 * Read a numeric value from a pointer, into the value union of a magic
 * pointer, according to the magic type.  Update the string pointer to point
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
static int
getvalue(struct magic *m, char **p)
{
	int slen;

	if (m->type == STRING) {
		*p = getstr(*p, m->value.s, sizeof(m->value.s), &slen);
		m->vallen = slen;
	} else if (m->reln != 'x')
		m->value.l = signextend(m, strtol(*p, p, 0));
	return 0;
}

/*
 * Convert a string containing C character escapes.  Stop at an unescaped
 * space or tab. Copy the converted version to "p", returning its length in
 * *slen. Return updated scan pointer as function result.
 */
static char *
getstr(register char *s, register char *p, int plen, int *slen)
{
	char *origs = s,
	*origp = p;
	char *pmax = p + plen - 1;
	register int c;
	register int val;

	while ((c = *s++) != '\0') {
		if (isspace((unsigned char) c))
			break;
		if (p >= pmax) {
			kdError(7018) << "String too long: " << origs << endl;
			break;
		}
		if (c == '\\') {
			switch (c = *s++) {

				case '\0':
					goto out;

				default:
					*p++ = (char) c;
					break;

				case 'n':
					*p++ = '\n';
					break;

				case 'r':
					*p++ = '\r';
					break;

				case 'b':
					*p++ = '\b';
					break;

				case 't':
					*p++ = '\t';
					break;

				case 'f':
					*p++ = '\f';
					break;

				case 'v':
					*p++ = '\v';
					break;

					/* \ and up to 3 octal digits */
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					val = c - '0';
					c = *s++;	/* try for 2 */
					if (c >= '0' && c <= '7') {
						val = (val << 3) | (c - '0');
						c = *s++;	/* try for 3 */
						if (c >= '0' && c <= '7')
							val = (val << 3) | (c - '0');
						else
							--s;
					} else
						--s;
					*p++ = (char) val;
					break;

					/* \x and up to 3 hex digits */
				case 'x':
					val = 'x';	/* Default if no digits */
					c = hextoint(*s++);	/* Get next char */
					if (c >= 0) {
						val = c;
						c = hextoint(*s++);
						if (c >= 0) {
							val = (val << 4) + c;
							c = hextoint(*s++);
							if (c >= 0) {
								val = (val << 4) + c;
							} else
								--s;
						} else
							--s;
					} else
						--s;
					*p++ = (char) val;
					break;
			}
		} else
			*p++ = (char) c;
	}
      out:
	*p = '\0';
	*slen = p - origp;
	return s;
}


/* Single hex char to int; -1 if not a hex char. */
static int
hextoint(int c)
{
	if (!isascii((unsigned char) c))
		return -1;
	if (isdigit((unsigned char) c))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c + 10 - 'a';
	if ((c >= 'A') && (c <= 'F'))
		return c + 10 - 'A';
	return -1;
}

/*
 * Convert the byte order of the data we are looking at
 */
static int
mconvert(union VALUETYPE *p, struct magic *m)
{
	char *rt;

	switch (m->type) {
		case BYTE:
			return 1;
		case STRING:
			/* Null terminate and eat the return */
			p->s[sizeof(p->s) - 1] = '\0';
			if ((rt = strchr(p->s, '\n')) != NULL)
				*rt = '\0';
			return 1;
#ifndef WORDS_BIGENDIAN
		case SHORT:
#endif
		case BESHORT:
			p->h = (short) ((p->hs[0] << 8) | (p->hs[1]));
			return 1;
#ifndef WORDS_BIGENDIAN
		case LONG:
		case DATE:
#endif
		case BELONG:
		case BEDATE:
			p->l = (long)
			    ((p->hl[0] << 24) | (p->hl[1] << 16) | (p->hl[2] << 8) | (p->hl[3]));
			return 1;
#ifdef WORDS_BIGENDIAN
		case SHORT:
#endif
		case LESHORT:
			p->h = (short) ((p->hs[1] << 8) | (p->hs[0]));
			return 1;
#ifdef WORDS_BIGENDIAN
		case LONG:
		case DATE:
#endif
		case LELONG:
		case LEDATE:
			p->l = (long)
			    ((p->hl[3] << 24) | (p->hl[2] << 16) | (p->hl[1] << 8) | (p->hl[0]));
			return 1;
		default:
			kdError(7018) << "mconvert: invalid type " << m->type << endl;
			return 0;
	}
}


static int
mget(union VALUETYPE *p, unsigned char *s, struct magic *m,
     int nbytes)
{
	long offset = m->offset;
// The file length might be < sizeof(union VALUETYPE) (David)
// -> pad with zeros (the 'file' command does it this way)
// Thanks to Stan Covington <stan@calderasystems.com> for detailed report
	if (offset + (int)sizeof(union VALUETYPE) > nbytes)
	{
	  int have = nbytes - offset;
	  memset(p, 0, sizeof(union VALUETYPE));
	  if (have > 0)
	    memcpy(p, s + offset, have);
	} else
	  memcpy(p, s + offset, sizeof(union VALUETYPE));

	if (!mconvert(p, m))
		return 0;

	if (m->flag & INDIR) {

		switch (m->in.type) {
			case BYTE:
				offset = p->b + m->in.offset;
				break;
			case SHORT:
				offset = p->h + m->in.offset;
				break;
			case LONG:
				offset = p->l + m->in.offset;
				break;
		}

		if (offset + (int)sizeof(union VALUETYPE) > nbytes)
			 return 0;

		memcpy(p, s + offset, sizeof(union VALUETYPE));

		if (!mconvert(p, m))
			return 0;
	}
	return 1;
}

static int
mcheck(union VALUETYPE *p, struct magic *m)
{
	register unsigned long l = m->value.l;
	register unsigned long v;
	int matched;

	if ((m->value.s[0] == 'x') && (m->value.s[1] == '\0')) {
		kdError(7018) << "BOINK" << endl;
		return 1;
	}
	switch (m->type) {
		case BYTE:
			v = p->b;
			break;

		case SHORT:
		case BESHORT:
		case LESHORT:
			v = p->h;
			break;

		case LONG:
		case BELONG:
		case LELONG:
		case DATE:
		case BEDATE:
		case LEDATE:
			v = p->l;
			break;

		case STRING:
			l = 0;
			/*
			 * What we want here is: v = strncmp(m->value.s, p->s,
			 * m->vallen); but ignoring any nulls.  bcmp doesn't give
			 * -/+/0 and isn't universally available anyway.
			 */
			v = 0;
			{
				register unsigned char *a = (unsigned char *) m->value.s;
				register unsigned char *b = (unsigned char *) p->s;
				register int len = m->vallen;
				ASSERT(len);

				while (--len >= 0)
					if ((v = *b++ - *a++) != 0)
						break;
			}
			break;
		default:
			kdError(7018) << "mcheck: invalid type " << m->type << endl;
			return 0;	/* NOTREACHED */
	}
#if 0
	debug("Before signextend %08x", v);
#endif
	v = signextend(m, v) & m->mask;
#if 0
	debug("After signextend %08x", v);
#endif

	switch (m->reln) {
		case 'x':
			matched = 1;
			break;

		case '!':
			matched = v != l;
			break;

		case '=':
			matched = v == l;
			break;

		case '>':
			if (m->flag & UNSIGNED)
				matched = v > l;
			else
				matched = (long) v > (long) l;
			break;

		case '<':
			if (m->flag & UNSIGNED)
				matched = v < l;
			else
				matched = (long) v < (long) l;
			break;

		case '&':
			matched = (v & l) == l;
			break;

		case '^':
			matched = (v & l) != l;
			break;

		default:
			matched = 0;
			kdError(7018) << "mcheck: can't happen: invalid relation " << m->reln << "." << endl;
			break;  /* NOTREACHED */
	}

	return matched;
}

#if 0

/* states for the state-machine algorithm in finishResult() */
typedef enum {
	rsl_leading_space, rsl_type, rsl_subtype, rsl_separator, rsl_encoding
} rsl_states;

/* process resultBuf and set the MIME info in magicResult */
int
KMimeMagic::finishResult()
{
	int cur_pos,            /* current position within result */
	 type_pos,              /* content type starting point: position */
	 type_len,              /* content type length */
	 encoding_pos,          /* content encoding starting point: position */
	 encoding_len;          /* content encoding length */

	int state;
	/* start searching for the type and encoding */
	state = rsl_leading_space;
	type_pos = type_len = 0;
	encoding_pos = encoding_len = 0;
        //kdDebug(7018) << "KMimeMagic::finishResult " << resultBuf << endl;
	/* loop through the characters in the result */
	for (cur_pos = 0; cur_pos < (int)resultBuf.length(); cur_pos++) {
		if (resultBuf[cur_pos].isSpace()) {
			/* process whitespace actions for each state */
			if (state == rsl_leading_space) {
				/* eat whitespace in this state */
				continue;
			} else if (state == rsl_type) {
				/* whitespace: type has no slash! */
				return DECLINED;
			} else if (state == rsl_subtype) {
				/* whitespace: end of MIME type */
				state++;
				continue;
			} else if (state == rsl_separator) {
				/* eat whitespace in this state */
				continue;
			} else if (state == rsl_encoding) {
				/* whitespace: end of MIME encoding */
				/* we're done */
				break;
			} else {
				/* should not be possible */
				/* abandon malfunctioning module */
				kdError(7018) << "KMimeMagic::finishResult: bad state " << state << " (ws)" << endl;
				return DECLINED;
			}
			/* NOTREACHED */
		} else if (state == rsl_type &&
			   resultBuf.at(cur_pos) == '/') {
			/* copy the char and go to rsl_subtype state */
			type_len++;
			state++;
		} else {
			/* process non-space actions for each state */
			if (state == rsl_leading_space) {
				/* non-space: begin MIME type */
				state++;
				type_pos = cur_pos;
				type_len = 1;
				continue;
			} else if (state == rsl_type ||
				   state == rsl_subtype) {
				/* non-space: adds to type */
				type_len++;
				continue;
			} else if (state == rsl_separator) {
				/* non-space: begin MIME encoding */
				state++;
				encoding_pos = cur_pos;
				encoding_len = 1;
				continue;
			} else if (state == rsl_encoding) {
				/* non-space: adds to encoding */
				encoding_len++;
				continue;
			} else {
				/* should not be possible */
				/* abandon malfunctioning module */
				kdError(7018) << " KMimeMagic::finishResult: bad state " << state << " (ns)" << endl;
				return DECLINED;
			}
			/* NOTREACHED */
		}
		/* NOTREACHED */
	}

	/* if we ended prior to state rsl_subtype, we had incomplete info */
	if (state != rsl_subtype && state != rsl_separator &&
	    state != rsl_encoding) {
		/* defer to other modules */
		return DECLINED;
	}
	/* save the info in the request record */
	if (state == rsl_subtype || state == rsl_encoding ||
	    state == rsl_encoding || state == rsl_separator) {
		magicResult->setMimeType(resultBuf.mid(type_pos, type_len).ascii());
	}
	if (state == rsl_encoding)
		magicResult->setEncoding(resultBuf.mid(encoding_pos,
						       encoding_len).ascii());
	/* detect memory allocation errors */
	if (!magicResult->mimeType() ||
	    (state == rsl_encoding && !magicResult->encoding())) {
		return -1;
	}
	/* success! */
	return OK;
}
#endif

/*
 * magic_process - process input file fn. Opens the file and reads a
 * fixed-size buffer to begin processing the contents.
 */
void
KMimeMagic::process(const QString & fn)
{
	int fd = 0;
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */
	struct stat sb;
	int nbytes = 0;         /* number of bytes read from a datafile */
        QCString fileName = QFile::encodeName( fn );

	/*
	 * first try judging the file based on its filesystem status
	 */
	if (fsmagic(fileName, &sb) != 0) {
		//resultBuf += "\n";
		return;
	}
	if ((fd = open(fileName, O_RDONLY)) < 0) {
		/* We can't open it, but we were able to stat it. */
		/*
		 * if (sb.st_mode & 0002) addResult("writable, ");
		 * if (sb.st_mode & 0111) addResult("executable, ");
		 */
		//kdDebug(7018) << "can't read `" << fn << "' (" << strerror(errno) << ")." << endl;
		resultBuf = MIME_BINARY_UNREADABLE;
		return;
	}
	/*
	 * try looking at the first HOWMANY bytes
	 */
	if ((nbytes = read(fd, (char *) buf, HOWMANY)) == -1) {
		kdError(7018) << "" << fn << " read failed (" << strerror(errno) << ")." << endl;
		resultBuf = MIME_BINARY_UNREADABLE;
		return;
		/* NOTREACHED */
	}
	if (nbytes == 0) {
		resultBuf = MIME_BINARY_ZEROSIZE;
	} else {
		buf[nbytes++] = '\0';	/* null-terminate it */
		tryit(buf, nbytes);
	}

        if ( conf->utimeConf && conf->utimeConf->restoreAccessTime( fn ) )
        {
            /*
             * Try to restore access, modification times if read it.
             * This changes the "change" time (ctime), but we can't do anything
             * about that.
             */
            struct utimbuf utbuf;
            utbuf.actime = sb.st_atime;
            utbuf.modtime = sb.st_mtime;
            (void) utime(fileName, &utbuf);
        }
	(void) close(fd);
}


void
KMimeMagic::tryit(unsigned char *buf, int nb)
{
	/* try tests in /etc/magic (or surrogate magic file) */
	if (match(buf, nb))
		return;

	/* try known keywords, check for ascii-ness too. */
	if (ascmagic(buf, nb) == 1)
		return;

        /* see if it's plain text */
        if (textmagic(buf, nb))
                return;

	/* abandon hope, all ye who remain here */
	resultBuf = MIME_BINARY_UNKNOWN;
	accuracy = 0;
}

int
KMimeMagic::fsmagic(const char *fn, struct stat *sb)
{
	int ret = 0;

	/*
	 * Fstat is cheaper but fails for files you don't have read perms on.
	 * On 4.2BSD and similar systems, use lstat() to identify symlinks.
	 */
	ret = lstat(fn, sb);  /* don't merge into if; see "ret =" above */

	if (ret) {
		return 1;
	}
	/*
	 * if (sb->st_mode & S_ISUID) resultBuf += "setuid ";
	 * if (sb->st_mode & S_ISGID) resultBuf += "setgid ";
	 * if (sb->st_mode & S_ISVTX) resultBuf += "sticky ";
	 */

	switch (sb->st_mode & S_IFMT) {
		case S_IFDIR:
			resultBuf = MIME_INODE_DIR;
			return 1;
		case S_IFCHR:
			resultBuf = MIME_INODE_CDEV;
			return 1;
		case S_IFBLK:
			resultBuf = MIME_INODE_BDEV;
			return 1;
			/* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef    S_IFIFO
		case S_IFIFO:
			resultBuf = MIME_INODE_FIFO;;
			return 1;
#endif
#ifdef    S_IFLNK
		case S_IFLNK:
			{
				char buf[BUFSIZ + BUFSIZ + 4];
				register int nch;
				struct stat tstatbuf;

				if ((nch = readlink(fn, buf, BUFSIZ - 1)) <= 0) {
					resultBuf = MIME_INODE_LINK;
					//resultBuf += "\nunreadable";
					return 1;
				}
				buf[nch] = '\0'; /* readlink(2) forgets this */
				/* If broken symlink, say so and quit early. */
				if (*buf == '/') {
					if (stat(buf, &tstatbuf) < 0) {
						resultBuf = MIME_INODE_LINK;
						//resultBuf += "\nbroken";
						return 1;
					}
				} else {
					char *tmp;
					char buf2[BUFSIZ + BUFSIZ + 4];

					strncpy(buf2, fn, BUFSIZ);
                    buf2[BUFSIZ] = 0;

					if ((tmp = strrchr(buf2, '/')) == NULL) {
						tmp = buf; /* in current dir */
					} else {
						/* dir part plus (rel.) link */
						*++tmp = '\0';
						strcat(buf2, buf);
						tmp = buf2;
					}
					if (stat(tmp, &tstatbuf) < 0) {
						resultBuf = MIME_INODE_LINK;
						//resultBuf += "\nbroken";
						return 1;
					} else
						strcpy(buf, tmp);
				}
				if (followLinks)
					process( QFile::decodeName( buf ) );
				else
					resultBuf = MIME_INODE_LINK;
				return 1;
			}
			return 1;
#endif
#ifdef    S_IFSOCK
#ifndef __COHERENT__
		case S_IFSOCK:
			resultBuf = MIME_INODE_SOCK;
			return 1;
#endif
#endif
		case S_IFREG:
			break;
		default:
			kdError(7018) << "KMimeMagic::fsmagic: invalid mode 0" << sb->st_mode << "." << endl;
			/* NOTREACHED */
	}

	/*
	 * regular file, check next possibility
	 */
	if (sb->st_size == 0) {
		resultBuf = MIME_BINARY_ZEROSIZE;
		return 1;
	}
	return 0;
}

/*
 * Go through the whole list, stopping if you find a match.  Process all the
 * continuations of that match before returning.
 *
 * We support multi-level continuations:
 *
 * At any time when processing a successful top-level match, there is a current
 * continuation level; it represents the level of the last successfully
 * matched continuation.
 *
 * Continuations above that level are skipped as, if we see one, it means that
 * the continuation that controls them - i.e, the lower-level continuation
 * preceding them - failed to match.
 *
 * Continuations below that level are processed as, if we see one, it means
 * we've finished processing or skipping higher-level continuations under the
 * control of a successful or unsuccessful lower-level continuation, and are
 * now seeing the next lower-level continuation and should process it.  The
 * current continuation level reverts to the level of the one we're seeing.
 *
 * Continuations at the current level are processed as, if we see one, there's
 * no lower-level continuation that may have failed.
 *
 * If a continuation matches, we bump the current continuation level so that
 * higher-level continuations are processed.
 */
int
KMimeMagic::match(unsigned char *s, int nbytes)
{
	int cont_level = 0;
	union VALUETYPE p;
	struct magic *m;

	//kdDebug(7018) << "match: conf=" << conf << " file=" << conf->magicfile.local8Bit().data() << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	for (m = conf->magic; m; m = m->next) {
		if (isprint((((unsigned long) m) >> 24) & 255) &&
		    isprint((((unsigned long) m) >> 16) & 255) &&
		    isprint((((unsigned long) m) >> 8) & 255) &&
		    isprint(((unsigned long) m) & 255)) {
			//kdDebug(7018) << "match: POINTER CLOBBERED! " << endl;
			break;
		}
	}

	for (m = conf->magic; m; m = m->next) {
		//kdDebug(7018) << "match: line=" << m->lineno << " desc=" << m->desc << endl;

		/* check if main entry matches */
		if (!mget(&p, s, m, nbytes) ||
		    !mcheck(&p, m)) {
			struct magic *m_cont;

			/*
			 * main entry didn't match, flush its continuations
			 */
			if (!m->next || (m->next->cont_level == 0)) {
				continue;
			}
			m_cont = m->next;
			while (m_cont && (m_cont->cont_level != 0)) {
				//kdDebug(7018) << "match: line=" << m->lineno << " cont=" << m_cont->cont_level << " mc=" << m_cont->lineno << " mc->next=" << m_cont << " " << endl;
				/*
				 * this trick allows us to keep *m in sync
				 * when the continue advances the pointer
				 */
				m = m_cont;
				m_cont = m_cont->next;
			}
			continue;
		}
		/* if we get here, the main entry rule was a match */
		/* this will be the last run through the loop */
		//kdDebug(7018) << "match: rule matched, line=" << m->lineno << " type=" << m->type << " " << ((m->type == STRING) ? m->value.s : "") << endl;

		/* remember the match */
		resultBuf = m->desc;

		cont_level++;
		/*
		 * while (m && m->next && m->next->cont_level != 0 && ( m =
		 * m->next ))
		 */
		m = m->next;
		while (m && (m->cont_level != 0)) {
			//kdDebug(7018) << "match: line=" << m->lineno << " cont=" << m->cont_level << " type=" << m->type << " " << ((m->type == STRING) ? m->value.s : "") << endl;
			if (cont_level >= m->cont_level) {
				if (cont_level > m->cont_level) {
					/*
					 * We're at the end of the level
					 * "cont_level" continuations.
					 */
					cont_level = m->cont_level;
				}
				if (mget(&p, s, m, nbytes) &&
				    mcheck(&p, m)) {
					/*
					 * This continuation matched. Print
					 * its message, with a blank before
					 * it if the previous item printed
					 * and this item isn't empty.
					 */
                                        //kdDebug() << "continuation matched" << endl;
					resultBuf = m->desc;
					cont_level++;
				}
			}
			/* move to next continuation record */
			m = m->next;
		}
		//kdDebug(7018) << "match: matched" << endl;
		return 1;       /* all through */
	}
	//kdDebug(7018) << "match: failed" << endl;
	return 0;               /* no match at all */
}

/* an optimization over plain strcmp() */
#define    STREQ(a, b)    (*(a) == *(b) && strcmp((a), (b)) == 0)

int
KMimeMagic::ascmagic(unsigned char *buf, int nbytes)
{
	int i;
	double pct, maxpct, pctsum;
	double pcts[NTYPES];
	int mostaccurate, tokencount;
	int typeset, jonly, conly, jconly, cppcomm, ccomm;
	int has_escapes = 0;
	unsigned char *s;
	char nbuf[HOWMANY + 1]; /* one extra for terminating '\0' */
	char *token;
	register struct names *p;
	int typecount[NTYPES];

	/* these are easy, do them first */
	accuracy = 70;

	/*
	 * for troff, look for . + letter + letter or .\"; this must be done
	 * to disambiguate tar archives' ./file and other trash from real
	 * troff input.
	 */
	if (*buf == '.') {
		unsigned char *tp = buf + 1;

		while (isascii(*tp) && isspace(*tp))
			++tp;   /* skip leading whitespace */
		if ((isascii(*tp) && (isalnum(*tp) || *tp == '\\') &&
		     isascii(*(tp + 1)) && (isalnum(*(tp + 1)) || *tp == '"'))) {
			resultBuf = MIME_APPL_TROFF;
			return 1;
		}
	}
	if ((*buf == 'c' || *buf == 'C') &&
	    isascii(*(buf + 1)) && isspace(*(buf + 1))) {
		/* Fortran */
		resultBuf = MIME_TEXT_FORTRAN;
		return 1;
	}
	assert(nbytes-1 < HOWMANY + 1);
	/* look for tokens - this is expensive! */
	/* make a copy of the buffer here because strtok() will destroy it */
	s = (unsigned char *) memcpy(nbuf, buf, nbytes);
	s[nbytes-1] = '\0';
	has_escapes = (memchr(s, '\033', nbytes) != NULL);
/*
 * Fritz:
 * Try a little harder on C/C++/Java.
 */
	memset(&typecount, 0, sizeof(typecount));
	typeset = 0;
	jonly = 0;
	conly = 0;
	jconly = 0;
	cppcomm = 0;
	ccomm = 0;
	tokencount = 0;
	// first collect all possible types and count matches
	while ((token = strtok((char *) s, " \t\n\r\f,;")) != NULL) {
		s = NULL;       /* make strtok() keep on tokin' */
#ifdef DEBUG_MIMEMAGIC
                kdDebug() << "KMimeMagic::ascmagic token=" << token << endl;
#endif
		for (p = names; p->name ; p++) {
			if (STREQ(p->name, token)) {
#ifdef DEBUG_MIMEMAGIC
                                kdDebug() << "KMimeMagic::ascmagic token matches ! name=" << p->name << " type=" << p->type << endl;
#endif
			        tokencount++;
				typeset |= p->type;
				if (p->type == L_JAVA)
					jonly++;
				if ((p->type & (L_C|L_CPP|L_JAVA))
				    == (L_CPP|L_JAVA))
					jconly++;
				if ((p->type & (L_C|L_CPP|L_JAVA))
				    == (L_C|L_CPP))
					conly++;
				if (STREQ(token, "//"))
					cppcomm++;
				if (STREQ(token, "/*"))
					ccomm++;
				for (i = 0; i < (int)NTYPES; i++)
					if ((1 << i) & p->type)
						typecount[i]++;
			}
		}
	}

	if (typeset & (L_C|L_CPP|L_JAVA)) {
		accuracy = 40;
	        if (!(typeset & ~(L_C|L_CPP|L_JAVA))) {
			if (jonly && conly)
                            // Take the biggest
                            if ( jonly > conly )
                                conly = 0;
                            else
                                jonly = 0;
			if (jonly) {
				// A java-only token has matched
				resultBuf = QString(types[P_JAVA].type);
				return 1;
			}
			if (jconly) {
				// A non-C (only C++ or Java) token has matched.
				if (typecount[P_JAVA] > typecount[P_CPP])
				  resultBuf = QString(types[P_JAVA].type);
				else
				  resultBuf = QString(types[P_CPP].type);
				return 1;
			}
			if (conly) {
				// Either C or C++, rely on comments.
				if (cppcomm)
				  resultBuf = QString(types[P_CPP].type);
				else
				  resultBuf = QString(types[P_C].type);
				return 1;
			}
			if (ccomm) {
				resultBuf = QString(types[P_C].type);
				return 1;
			}
	      }
	}

	/* Neither C, C++ or Java (or all of them without able to distinguish):
	 * Simply take the token-class with the highest
	 * matchcount > 0
	 */
	mostaccurate = -1;
	maxpct = pctsum = 0.0;
	for (i = 0; i < (int)NTYPES; i++) {
	  if (typecount[i]) {
		pct = (double)typecount[i] / (double)types[i].kwords *
		    (double)types[i].weight;
		pcts[i] = pct;
		pctsum += pct;
		if (pct > maxpct) {
		    maxpct = pct;
		    mostaccurate = i;
		  }
#ifdef DEBUG_MIMEMAGIC
		  kdDebug(7018) << "" << types[i].type << " has " << typecount[i] << " hits, " << types[i].kwords << " kw, weight " << types[i].weight << ", " << pct << " -> max = " << maxpct << "\n" << endl;
#endif
	  }
	}
	if (mostaccurate >= 0.0) {
		accuracy = (int)(pcts[mostaccurate] / pctsum * 60);
		resultBuf = QString(types[mostaccurate].type);
		return 1;
	}

	switch (is_tar(buf, nbytes)) {
		case 1:
			/* V7 tar archive */
			resultBuf = MIME_APPL_TAR;
			accuracy = 90;
			return 1;
		case 2:
			/* POSIX tar archive */
			resultBuf = MIME_APPL_TAR;
			accuracy = 90;
			return 1;
	}

	for (i = 0; i < nbytes; i++) {
		if (!isascii(*(buf + i)))
			return 0;	/* not all ascii */
	}

	/* all else fails, but it is ascii... */
	accuracy = 90;
	if (has_escapes) {
		/* text with escape sequences */
		/* we leave this open for further differentiation later */
		resultBuf = MIME_TEXT_UNKNOWN;
	} else {
		/* plain text */
		resultBuf = MIME_TEXT_PLAIN;
	}
	return 1;
}

/* Maximal length of a line we consider "reasonable". */
#define TEXT_MAXLINELEN 300

// This code is taken from the "file" command, where it is licensed
// in the "beer-ware license" :-)
// Original author: <joerg@FreeBSD.ORG>
// Simplified by David Faure to avoid the static array char[256].
int KMimeMagic::textmagic(unsigned char * buf, int nbytes)
{
    int i;
    unsigned char *cp;

    nbytes--;

    /* First, look whether there are "unreasonable" characters. */
    for (i = 0, cp = buf; i < nbytes; i++, cp++)
        if ((*cp < 8) || (*cp>13 && *cp<32 && *cp!=27 ) || (*cp==0x7F))
            return 0;

    /* Now, look whether the file consists of lines of
     * "reasonable" length. */

    for (i = 0; i < nbytes;) {
        cp = (unsigned char *) memchr(buf, '\n', nbytes - i);
        if (cp == NULL) {
            /* Don't fail if we hit the end of buffer. */
            if (i + TEXT_MAXLINELEN >= nbytes)
                break;
            else
                return 0;
        }
        if (cp - buf > TEXT_MAXLINELEN)
            return 0;
        i += (cp - buf + 1);
        buf = cp + 1;
    }
    resultBuf = MIME_TEXT_PLAIN;
    return 1;
}


/*
 * is_tar() -- figure out whether file is a tar archive.
 *
 * Stolen (by author of file utility) from the public domain tar program: Public
 * Domain version written 26 Aug 1985 John Gilmore (ihnp4!hoptoad!gnu).
 *
 * @(#)list.c 1.18 9/23/86 Public Domain - gnu $Id: mod_mime_magic.c,v 1.7
 * 1997/06/24 00:41:02 ikluft Exp ikluft $
 *
 * Comments changed and some code/comments reformatted for file command by Ian
 * Darwin.
 */

#define    isodigit(c)    ( ((c) >= '0') && ((c) <= '7') )

/*
 * Return 0 if the checksum is bad (i.e., probably not a tar archive), 1 for
 * old UNIX tar file, 2 for Unix Std (POSIX) tar file.
 */

static int
is_tar(unsigned char *buf, int nbytes)
{
	register union record *header = (union record *) buf;
	register int i;
	register long sum,
	 recsum;
	register char *p;

	if (nbytes < (int)sizeof(union record))
		 return 0;

	recsum = from_oct(8, header->header.chksum);

	sum = 0;
	p = header->charptr;
	for (i = sizeof(union record); --i >= 0;) {
		/*
		 * We can't use unsigned char here because of old compilers,
		 * e.g. V7.
		 */
		sum += 0xFF & *p++;
	}

	/* Adjust checksum to count the "chksum" field as blanks. */
	for (i = sizeof(header->header.chksum); --i >= 0;)
		sum -= 0xFF & header->header.chksum[i];
	sum += ' ' * sizeof header->header.chksum;

	if (sum != recsum)
		return 0;       /* Not a tar archive */

	if (0 == strcmp(header->header.magic, TMAGIC))
		return 2;       /* Unix Standard tar archive */

	return 1;               /* Old fashioned tar archive */
}


/*
 * Quick and dirty octal conversion.
 *
 * Result is -1 if the field is invalid (all blank, or nonoctal).
 */
static long
from_oct(int digs, char *where)
{
	register long value;

	while (isspace(*where)) {	/* Skip spaces */
		where++;
		if (--digs <= 0)
			return -1;	/* All blank field */
	}
	value = 0;
	while (digs > 0 && isodigit(*where)) {	/* Scan til nonoctal */
		value = (value << 3) | (*where++ - '0');
		--digs;
	}

	if (digs > 0 && *where && !isspace(*where))
		return -1;      /* Ended on non-space/nul */

	return value;
}

/*
 * The Constructor
 */
KMimeMagic::KMimeMagic(const QString & _configfile)
{
	int result;
	conf = new config_rec;

	/* set up the magic list (empty) */
	conf->magic = conf->last = NULL;
	magicResult = NULL;
	followLinks = FALSE;

	conf->magicfile = _configfile;
        conf->utimeConf = 0L; // created on demand
	/* on the first time through we read the magic file */
	result = apprentice();
	if (result == -1)
		return;
#if (MIME_MAGIC_DEBUG_TABLE > 1)
	test_table();
#endif
}

/*
 * The destructor.
 * Free the magic-table and other resources.
 */
KMimeMagic::~KMimeMagic()
{
	if (conf) {
		struct magic *p = conf->magic;
		struct magic *q;
		while (p) {
			q = p;
			p = p->next;
			free(q);
		}
                delete conf->utimeConf;
		delete conf;
	}
        delete magicResult;
}

bool
KMimeMagic::mergeConfig(const QString & _configfile)
{
	int result;

	QString old_magicfile = conf->magicfile;

	if (!_configfile.isEmpty())
		conf->magicfile = _configfile;
	else
		return false;
	result = apprentice();
	if (result == -1) {
		conf->magicfile = old_magicfile;
		return false;
	}
#if (MIME_MAGIC_DEBUG_TABLE > 1)
	test_table();
#endif
	return true;
}

bool
KMimeMagic::mergeBufConfig(char * _configbuf)
{
	int result;

	if (conf) {
		result = buff_apprentice(_configbuf);
		if (result == -1)
			return false;
#if (MIME_MAGIC_DEBUG_TABLE > 1)
		test_table();
#endif
		return true;
	}
	return false;
}

void
KMimeMagic::setFollowLinks( bool _enable )
{
	followLinks = _enable;
}

KMimeMagicResult *
KMimeMagic::findBufferType(const QByteArray &array)
{
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */

	resultBuf = QString::null;
	if ( !magicResult )
	  magicResult = new KMimeMagicResult();
	magicResult->setInvalid();
	accuracy = 100;

	int nbytes = array.size();

        if (nbytes > HOWMANY)
                nbytes = HOWMANY;
        memcpy(buf, array.data(), nbytes);
        if (nbytes == 0) {
                resultBuf = MIME_BINARY_ZEROSIZE;
        } else {
                buf[nbytes++] = '\0';   /* null-terminate it */
                tryit(buf, nbytes);
        }
        /* if we have any results, put them in the request structure */
        //finishResult();
	magicResult->setMimeType(resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(accuracy);
        return magicResult;
}

static void
refineResult(KMimeMagicResult *r, const QString & _filename)
{
	QString tmp = r->mimeType();
	if (tmp.isEmpty())
		return;
	if ( tmp == "text/x-c"  ||
	     tmp == "text/x-c++" )
	{
		if ( _filename.right(2) == ".h" )
			tmp += "hdr";
		else
			tmp += "src";
		r->setMimeType(tmp);
	}
}

KMimeMagicResult *
KMimeMagic::findBufferFileType( const QByteArray &data,
				const QString &fn)
{
        KMimeMagicResult * r = findBufferType( data );
	refineResult(r, fn);
        return r;
}

/*
 * Find the content-type of the given file.
 */
KMimeMagicResult* KMimeMagic::findFileType(const QString & fn)
{
        resultBuf = QString::null;

        if ( !magicResult )
	  magicResult = new KMimeMagicResult();
	magicResult->setInvalid();
	accuracy = 100;

        if ( !conf->utimeConf )
            conf->utimeConf = new KMimeMagicUtimeConf();

        /* process it based on the file contents */
        process( fn );

        /* if we have any results, put them in the request structure */
        //finishResult();
	magicResult->setMimeType(resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(accuracy);
	refineResult(magicResult, fn);
        return magicResult;
}
