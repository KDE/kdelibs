/* This file is part of the KDE libraries
   Copyright (C) 2000 Fritz Elfert <fritz@kde.org>
   Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmimemagic.h"
#include <kdebug.h>
#include <kapplication.h>
#include <qfile.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kde_file.h>
#include <assert.h>

static int fsmagic(struct config_rec* conf, const char *fn, KDE_struct_stat *sb);
static void process(struct config_rec* conf,  const QString &);
static int ascmagic(struct config_rec* conf, unsigned char *buf, int nbytes);
static int tagmagic(unsigned char *buf, int nbytes);
static int textmagic(struct config_rec* conf, unsigned char *, int);

static void tryit(struct config_rec* conf, unsigned char *buf, int nb);
static int match(struct config_rec* conf, unsigned char *, int);

KMimeMagic* KMimeMagic::s_pSelf;
static KStaticDeleter<KMimeMagic> kmimemagicsd;

KMimeMagic* KMimeMagic::self()
{
  if( !s_pSelf )
    initStatic();
  return s_pSelf;
}

void KMimeMagic::initStatic()
{
  s_pSelf = kmimemagicsd.setObject( s_pSelf, new KMimeMagic() );
  s_pSelf->setFollowLinks( true );
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

//#define MIME_MAGIC_DEBUG_TABLE // untested

// Uncomment to debug the config-file parsing phase
//#define DEBUG_APPRENTICE
// Uncomment to debug the matching phase
//#define DEBUG_MIMEMAGIC

#if (defined DEBUG_MIMEMAGIC || defined DEBUG_APPRENTICE)
#define DEBUG_LINENUMBERS
#endif

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

#define HOWMANY 4000            /* big enough to recognize most WWW files, and skip GPL-headers */
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
#ifdef DEBUG_LINENUMBERS
	int lineno;             /* line number from magic file - doesn't say from which one ;) */
#endif

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
	char nospflag;          /* suppress space character */

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
#define L_OBJC   0x800          /* Objective C */

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
#define P_OBJC  11          /* Objective C */

typedef struct asc_type {
	const char *type;
	int  kwords;
	double  weight;
} asc_type;

static const asc_type types[] = {
	{ "text/html",         19, 2 }, // 10 items but 10 different words only
	{ "text/x-c",          13, 1 },
	{ "text/x-makefile",    4, 1.9 },
	{ "text/x-pli",         1, 3 },
	{ "text/x-assembler",   6, 2.1 },
	{ "text/x-pascal",      1, 1 },
	{ "text/x-java",       12, 1 },
	{ "text/x-c++",        19, 1 },
	{ "message/rfc822",     4, 1.9 },
	{ "message/news",       3, 2 },
        { "text/x-diff",        4, 2 },
        { "text/x-objc",    10, 1 }
};

#define NTYPES (sizeof(types)/sizeof(asc_type))

static struct names {
	const char *name;
	short type;
} const names[] = {
	{
		"<html", L_HTML
	},
	{
		"<HTML", L_HTML
	},
	{
		"<head", L_HTML
	},
	{
		"<HEAD", L_HTML
	},
	{
		"<body", L_HTML
	},
	{
		"<BODY", L_HTML
	},
	{
		"<title", L_HTML
	},
	{
		"<TITLE", L_HTML
	},
	{
		"<h1", L_HTML
	},
	{
		"<H1", L_HTML
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
		"<frame", L_HTML
	},
	{
		"<FRAME", L_HTML
	},
	{
		"<frameset", L_HTML
	},
	{
		"<FRAMESET", L_HTML
	},
        {
                "<script", L_HTML
        },
        {
                "<SCRIPT", L_HTML
        },
	{
		"/*", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"//", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"#include", L_C|L_CPP
	},
	{
		"#ifdef", L_C|L_CPP
	},
	{
		"#ifndef", L_C|L_CPP
	},
	{
		"bool", L_C|L_CPP
	},
	{
		"char", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"int", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"float", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"void", L_C|L_CPP|L_JAVA|L_OBJC
	},
	{
		"extern", L_C|L_CPP
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
		"explicit", L_CPP
	},
	{
		"virtual", L_CPP
	},
	{
		"namespace", L_CPP
	},
	{
		"#import", L_OBJC
	},
	{
		"@interface", L_OBJC
	},
	{
		"@implementation", L_OBJC
	},
	{
		"@protocol", L_OBJC
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
		".PHONY:", L_MAKE
	},
	{
		"srcdir", L_MAKE
	},
	{
		"exec_prefix", L_MAKE
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
	/* we now stop at '>' for tokens, so this one won't work {
		">From", L_MAIL
        },*/
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
        tmpDirs << QLatin1String("/tmp"); // default value

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
            kdDebug(7018) << " atimeDir: " << *it << endl;
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
    bool followLinks;
    QString resultBuf;
    int accuracy;

    struct magic *magic,    /* head of magic config list */
	*last;
    KMimeMagicUtimeConf * utimeConf;
};

#ifdef MIME_MAGIC_DEBUG_TABLE
static void
test_table()
{
	struct magic *m;
	struct magic *prevm = NULL;

	kdDebug(7018) << "test_table : started" << endl;
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
int KMimeMagic::apprentice( const QString& magicfile )
{
	FILE *f;
	char line[BUFSIZ + 1];
	int errs = 0;
	int lineno;
	int rule = 0;
	QByteArray fname;

	if (magicfile.isEmpty())
		return -1;
	fname = QFile::encodeName(magicfile);
	f = fopen(fname.constData(), "r");
	if (f == NULL) {
		kdError(7018) << "can't read magic file " << fname.constData() << ": " << strerror(errno) << endl;
		return -1;
	}

	/* parse it */
	for (lineno = 1; fgets(line, BUFSIZ, f) != NULL; lineno++)
		if (parse_line(line, &rule, lineno))
			errs++;

	fclose(f);

#ifdef DEBUG_APPRENTICE
	kdDebug(7018) << "apprentice: conf=" << conf << " file=" << magicfile << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	kdDebug(7018) << "apprentice: read " << lineno << " lines, " << rule << " rules, " << errs << " errors" << endl;
#endif

#ifdef MIME_MAGIC_DEBUG_TABLE
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

#ifdef DEBUG_APPRENTICE
	kdDebug(7018) << "buff_apprentice: conf=" << conf << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	kdDebug(7018) << "buff_apprentice: read " << lineno << " lines, " << rule << " rules, " << errs << " errors" << endl;
#endif

#ifdef MIME_MAGIC_DEBUG_TABLE
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
				return 998; //good value
		}
	return v;
}

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
int KMimeMagic::parse(char *l, int
#ifdef DEBUG_LINENUMBERS
    lineno
#endif
        )
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
#ifdef DEBUG_LINENUMBERS
	m->lineno = lineno;
#endif

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
        // Copy description - until EOL or '#' (for comments)
        while (*l != '\0' && *l != '#' && i < MAXDESC-1)
            m->desc[i++] = *l++;
        m->desc[i] = '\0';
        // Remove trailing spaces
        while (--i>0 && isspace( m->desc[i] ))
            m->desc[i] = '\0';

        // old code
	//while ((m->desc[i++] = *l++) != '\0' && i < MAXDESC) /* NULLBODY */ ;

#ifdef DEBUG_APPRENTICE
	kdDebug(7018) << "parse: line=" << lineno << " m=" << m << " next=" << m->next << " cont=" << m->cont_level << " desc=" << (m->desc ? m->desc : "NULL") << endl;
#endif
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
	//for ( char* foo = origp; foo < p ; ++foo )
	//	kdDebug(7018) << "  " << *foo << endl;
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
	switch (m->type) {
		case BYTE:
			return 1;
		case STRING:
			/* Null terminate */
			p->s[sizeof(p->s) - 1] = '\0';
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
        switch ( m->type )
	{
	    case BYTE:
		if ( offset + 1 > nbytes-1 ) // nbytes = (size of file) + 1
		    return 0;
		break;
	    case SHORT:
	    case BESHORT:
	    case LESHORT:
	       	if ( offset + 2 > nbytes-1 )
		    return 0;
		break;
	    case LONG:
	    case BELONG:
	    case LELONG:
	    case DATE:
	    case BEDATE:
	    case LEDATE:
	       	if ( offset + 4 > nbytes-1 )
		    return 0;
		break;
	    case STRING:
		break;
	}

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
				Q_ASSERT(len);

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
	qDebug("Before signextend %08x", v);
#endif
	v = signextend(m, v) & m->mask;
#if 0
	qDebug("After signextend %08x", v);
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

/*
 * magic_process - process input file fn. Opens the file and reads a
 * fixed-size buffer to begin processing the contents.
 */

void process(struct config_rec* conf, const QString & fn)
{
	int fd = 0;
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */
	KDE_struct_stat sb;
	int nbytes = 0;         /* number of bytes read from a datafile */
        int tagbytes = 0;       /* size of prefixed tag */
        QByteArray fileName = QFile::encodeName( fn );

	/*
	 * first try judging the file based on its filesystem status
	 */
	if (fsmagic(conf, fileName.constData(), &sb) != 0) {
		//resultBuf += "\n";
		return;
	}
	if ((fd = KDE_open(fileName.constData(), O_RDONLY)) < 0) {
		/* We can't open it, but we were able to stat it. */
		/*
		 * if (sb.st_mode & 0002) addResult("writable, ");
		 * if (sb.st_mode & 0111) addResult("executable, ");
		 */
		//kdDebug(7018) << "can't read `" << fn << "' (" << strerror(errno) << ")." << endl;
		conf->resultBuf = MIME_BINARY_UNREADABLE;
		return;
	}
	/*
	 * try looking at the first HOWMANY bytes
	 */
	if ((nbytes = read(fd, (char *) buf, HOWMANY)) == -1) {
		kdError(7018) << "" << fn << " read failed (" << strerror(errno) << ")." << endl;
		conf->resultBuf = MIME_BINARY_UNREADABLE;
		return;
	}
        if ((tagbytes = tagmagic(buf, nbytes))) {
		// Read buffer at new position
		lseek(fd, tagbytes, SEEK_SET);
		nbytes = read(fd, (char*)buf, HOWMANY);
		if (nbytes < 0) {
			conf->resultBuf = MIME_BINARY_UNREADABLE;
			return;
		}
        }
	if (nbytes == 0) {
		conf->resultBuf = MIME_BINARY_ZEROSIZE;
	} else {
		buf[nbytes++] = '\0';	/* null-terminate it */
		tryit(conf, buf, nbytes);
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
            (void) utime(fileName.constData(), &utbuf);
        }
	(void) close(fd);
}


static void tryit(struct config_rec* conf, unsigned char *buf, int nb)
{
	/* try tests in /etc/magic (or surrogate magic file) */
	if (match(conf, buf, nb))
		return;

	/* try known keywords, check for ascii-ness too. */
	if (ascmagic(conf, buf, nb) == 1)
		return;

        /* see if it's plain text */
        if (textmagic(conf, buf, nb))
                return;

	/* abandon hope, all ye who remain here */
	conf->resultBuf = MIME_BINARY_UNKNOWN;
	conf->accuracy = 0;
}

static int
fsmagic(struct config_rec* conf, const char *fn, KDE_struct_stat *sb)
{
    int ret = 0;

    /*
     * Fstat is cheaper but fails for files you don't have read perms on.
     * On 4.2BSD and similar systems, use lstat() to identify symlinks.
     */
    ret = KDE_lstat(fn, sb);  /* don't merge into if; see "ret =" above */

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
        conf->resultBuf = MIME_INODE_DIR;
        return 1;
    case S_IFCHR:
        conf->resultBuf = MIME_INODE_CDEV;
        return 1;
    case S_IFBLK:
        conf->resultBuf = MIME_INODE_BDEV;
        return 1;
        /* TODO add code to handle V7 MUX and Blit MUX files */
#ifdef    S_IFIFO
    case S_IFIFO:
        conf->resultBuf = MIME_INODE_FIFO;
        return 1;
#endif
#ifdef    S_IFLNK
    case S_IFLNK:
    {
        char buf[BUFSIZ + BUFSIZ + 4];
        register int nch;
        KDE_struct_stat tstatbuf;

        if ((nch = readlink(fn, buf, BUFSIZ - 1)) <= 0) {
            conf->resultBuf = MIME_INODE_LINK;
            //conf->resultBuf += "\nunreadable";
            return 1;
        }
        buf[nch] = '\0'; /* readlink(2) forgets this */
        /* If broken symlink, say so and quit early. */
        if (*buf == '/') {
            if (KDE_stat(buf, &tstatbuf) < 0) {
                conf->resultBuf = MIME_INODE_LINK;
                //conf->resultBuf += "\nbroken";
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
            if (KDE_stat(tmp, &tstatbuf) < 0) {
                conf->resultBuf = MIME_INODE_LINK;
                //conf->resultBuf += "\nbroken";
                return 1;
            } else
                strcpy(buf, tmp);
        }
        if (conf->followLinks)
            process( conf, QFile::decodeName( buf ) );
        else
            conf->resultBuf = MIME_INODE_LINK;
        return 1;
    }
    return 1;
#endif
#ifdef    S_IFSOCK
#ifndef __COHERENT__
    case S_IFSOCK:
        conf->resultBuf = MIME_INODE_SOCK;
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
        conf->resultBuf = MIME_BINARY_ZEROSIZE;
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
static int
match(struct config_rec* conf, unsigned char *s, int nbytes)
{
	int cont_level = 0;
	union VALUETYPE p;
	struct magic *m;

#ifdef DEBUG_MIMEMAGIC
	kdDebug(7018) << "match: conf=" << conf << " m=" << (conf->magic ? "set" : "NULL") << " m->next=" << ((conf->magic && conf->magic->next) ? "set" : "NULL") << " last=" << (conf->last ? "set" : "NULL") << endl;
	for (m = conf->magic; m; m = m->next) {
		if (isprint((((unsigned long) m) >> 24) & 255) &&
		    isprint((((unsigned long) m) >> 16) & 255) &&
		    isprint((((unsigned long) m) >> 8) & 255) &&
		    isprint(((unsigned long) m) & 255)) {
			kdDebug(7018) << "match: POINTER CLOBBERED! " << endl;
			break;
		}
	}
#endif

	for (m = conf->magic; m; m = m->next) {
#ifdef DEBUG_MIMEMAGIC
		kdDebug(7018) << "match: line=" << m->lineno << " desc=" << m->desc << endl;
#endif
		memset(&p, 0, sizeof(union VALUETYPE));

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
#ifdef DEBUG_MIMEMAGIC
				kdDebug(7018) << "match: line=" << m->lineno << " cont=" << m_cont->cont_level << " mc=" << m_cont->lineno << " mc->next=" << m_cont << " " << endl;
#endif
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
#ifdef DEBUG_MIMEMAGIC
		kdDebug(7018) << "match: rule matched, line=" << m->lineno << " type=" << m->type << " " << ((m->type == STRING) ? m->value.s : "") << endl;
#endif

		/* remember the match */
		conf->resultBuf = m->desc;

		cont_level++;
		/*
		 * while (m && m->next && m->next->cont_level != 0 && ( m =
		 * m->next ))
		 */
		m = m->next;
		while (m && (m->cont_level != 0)) {
#ifdef DEBUG_MIMEMAGIC
                    kdDebug(7018) << "match: line=" << m->lineno << " cont=" << m->cont_level << " type=" << m->type << " " << ((m->type == STRING) ? m->value.s : "") << endl;
#endif
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
#ifdef DEBUG_MIMEMAGIC
                                    kdDebug(7018) << "continuation matched" << endl;
#endif
                                    conf->resultBuf = m->desc;
					cont_level++;
				}
			}
			/* move to next continuation record */
			m = m->next;
		}
                // KDE-specific: need an actual mimetype for a real match
                // If we only matched a rule with continuations but no mimetype, it's not a match
                if ( !conf->resultBuf.isEmpty() )
                {
#ifdef DEBUG_MIMEMAGIC
                    kdDebug(7018) << "match: matched" << endl;
#endif
                    return 1;       /* all through */
                }
	}
#ifdef DEBUG_MIMEMAGIC
	kdDebug(7018) << "match: failed" << endl;
#endif
	return 0;               /* no match at all */
}

// Try to parse prefixed tags before matching on content
// Sofar only ID3v2 tags (<=.4) are handled
static int tagmagic(unsigned char *buf, int nbytes)
{
	if(nbytes<40) return 0;
	if(buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
		int size = 10;
		// Sanity (known version, no unknown flags)
		if(buf[3] > 4) return 0;
		if(buf[5] & 0x0F) return 0;
		// Tag has v4 footer
		if(buf[5] & 0x10) size += 10;
		// Calculated syncsafe size
		size += buf[9];
		size += buf[8] << 7;
		size += buf[7] << 14;
		size += buf[6] << 21;
		return size;
	}
	return 0;
}

struct Token {
    char *data;
    int length;
};

struct Tokenizer
{
    Tokenizer(char* buf, int nbytes) {
        data = buf;
        length = nbytes;
        pos = 0;
    }
    bool isNewLine() {
        return newline;
    }
    Token* nextToken() {
        if (pos == 0)
            newline = true;
        else
            newline = false;
        token.data = data+pos;
        token.length = 0;
        while(pos<length) {
            switch (data[pos]) {
                case '\n':
                    newline = true;
                case '\0':
                case '\t':
                case ' ':
                case '\r':
                case '\f':
                case ',':
                case ';':
                case '>':
                    if (token.length == 0) token.data++;
                    else
                        return &token;
                    break;
                default:
                    token.length++;
            }
            pos++;
        }
        return &token;
    }

private:
    Token token;
    char* data;
    int length;
    int pos;
    bool newline;
};


/* an optimization over plain strcmp() */
//#define    STREQ(a, b)    (*(a) == *(b) && strcmp((a), (b)) == 0)
static inline bool STREQ(const Token *token, const char *b) {
    const char *a = token->data;
    int len = token->length;
    if (a == b) return true;
    while(*a && *b && len > 0) {
        if (*a != *b) return false;
        a++; b++; len--;
    }
    return (len == 0 && *b == 0);
}

static int ascmagic(struct config_rec* conf, unsigned char *buf, int nbytes)
{
	int i;
	double pct, maxpct, pctsum;
	double pcts[NTYPES];
	int mostaccurate, tokencount;
	int typeset, jonly, conly, jconly, objconly, cpponly;
	int has_escapes = 0;
	//unsigned char *s;
	//char nbuf[HOWMANY + 1]; /* one extra for terminating '\0' */

	/* these are easy, do them first */
	conf->accuracy = 70;

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
			conf->resultBuf = MIME_APPL_TROFF;
			return 1;
		}
	}
	if ((*buf == 'c' || *buf == 'C') &&
	    isascii(*(buf + 1)) && isspace(*(buf + 1))) {
		/* Fortran */
		conf->resultBuf = MIME_TEXT_FORTRAN;
		return 1;
	}
	assert(nbytes-1 < HOWMANY + 1);
	/* look for tokens - this is expensive! */
	has_escapes = (memchr(buf, '\033', nbytes) != NULL);
        Tokenizer tokenizer((char*)buf, nbytes);
        const Token* token;
        bool linecomment = false, blockcomment = false;
	const struct names *p;
	int typecount[NTYPES];
/*
 * Fritz:
 * Try a little harder on C/C++/Java.
 */
	memset(&typecount, 0, sizeof(typecount));
	typeset = 0;
	jonly = 0;
	conly = 0;
	jconly = 0;
	objconly = 0;
	cpponly = 0;
	tokencount = 0;
        bool foundClass = false; // mandatory for java
	// first collect all possible types and count matches
        // we stop at '>' too, because of "<title>blah</title>" on HTML pages
	while ((token = tokenizer.nextToken())->length > 0) {
#ifdef DEBUG_MIMEMAGIC
            kdDebug(7018) << "KMimeMagic::ascmagic token=" << token << endl;
#endif
            if (linecomment && tokenizer.isNewLine())
                linecomment = false;
            if (blockcomment && STREQ(token, "*/")) {
                blockcomment = false;
                continue;
            }
            for (p = names; p->name ; p++) {
                if (STREQ(token, p->name)) {
#ifdef DEBUG_MIMEMAGIC
                    kdDebug(7018) << "KMimeMagic::ascmagic token matches ! name=" << p->name << " type=" << p->type << endl;
#endif
                    tokencount++;
                    typeset |= p->type;
                    if(p->type & (L_C|L_CPP|L_JAVA|L_OBJC)) {
                        if (linecomment || blockcomment) {
                            continue;
                        }
                        else {
                            switch(p->type & (L_C|L_CPP|L_JAVA|L_OBJC))
                            {
				case L_JAVA:
					jonly++;
					break;
				case L_OBJC:
					objconly++;
					break;
				case L_CPP:
					cpponly++;
					break;
				case (L_CPP|L_JAVA):
					jconly++;
                                        if ( !foundClass && STREQ(token, "class") )
                                            foundClass = true;
					break;
				case (L_C|L_CPP):
					conly++;
					break;
				default:
                                    if (STREQ(token, "//")) linecomment = true;
                                    if (STREQ(token, "/*")) blockcomment = true;
                            }
			}
                    }
                    for (i = 0; i < (int)NTYPES; i++) {
                        if ((1 << i) & p->type) typecount[i]++;
                    }
		}
            }
	}

	if (typeset & (L_C|L_CPP|L_JAVA|L_OBJC)) {
		conf->accuracy = 60;
	        if (!(typeset & ~(L_C|L_CPP|L_JAVA|L_OBJC))) {
#ifdef DEBUG_MIMEMAGIC
                        kdDebug(7018) << "C/C++/Java/ObjC: jonly=" << jonly << " conly=" << conly << " jconly=" << jconly << " objconly=" << objconly << endl;
#endif
			if (jonly > 1 && foundClass) {
				// At least two java-only tokens have matched, including "class"
				conf->resultBuf = QString(types[P_JAVA].type);
				return 1;
			}
			if (jconly > 1) {
				// At least two non-C (only C++ or Java) token have matched.
				if (typecount[P_JAVA] < typecount[P_CPP])
				  conf->resultBuf = QString(types[P_CPP].type);
				else
				  conf->resultBuf = QString(types[P_JAVA].type);
				return 1;
			}
                        if (conly + cpponly > 1) {
			     // Either C or C++.
        		      if (cpponly > 0)
                                conf->resultBuf = QString(types[P_CPP].type);
                              else
                                conf->resultBuf = QString(types[P_C].type);
                              return 1;
                        }
			if (objconly > 0) {
				conf->resultBuf =  QString(types[P_OBJC].type);
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
	  if (typecount[i] > 1) { // one word is not enough, we need at least two
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
	if (mostaccurate >= 0) {
            if ( mostaccurate != P_JAVA || foundClass ) // 'class' mandatory for java
            {
		conf->accuracy = (int)(pcts[mostaccurate] / pctsum * 60);
#ifdef DEBUG_MIMEMAGIC
                kdDebug(7018) << "mostaccurate=" << mostaccurate << " pcts=" << pcts[mostaccurate] << " pctsum=" << pctsum << " accuracy=" << conf->accuracy << endl;
#endif
		conf->resultBuf = QString(types[mostaccurate].type);
		return 1;
            }
	}

	switch (is_tar(buf, nbytes)) {
		case 1:
			/* V7 tar archive */
			conf->resultBuf = MIME_APPL_TAR;
			conf->accuracy = 90;
			return 1;
		case 2:
			/* POSIX tar archive */
			conf->resultBuf = MIME_APPL_TAR;
			conf->accuracy = 90;
			return 1;
	}

	for (i = 0; i < nbytes; i++) {
		if (!isascii(*(buf + i)))
			return 0;	/* not all ascii */
	}

	/* all else fails, but it is ascii... */
	conf->accuracy = 90;
	if (has_escapes) {
		/* text with escape sequences */
		/* we leave this open for further differentiation later */
		conf->resultBuf = MIME_TEXT_UNKNOWN;
	} else {
		/* plain text */
		conf->resultBuf = MIME_TEXT_PLAIN;
	}
	return 1;
}

/* Maximal length of a line we consider "reasonable". */
#define TEXT_MAXLINELEN 300

// This code is taken from the "file" command, where it is licensed
// in the "beer-ware license" :-)
// Original author: <joerg@FreeBSD.ORG>
// Simplified by David Faure to avoid the static array char[256].
static int textmagic(struct config_rec* conf, unsigned char * buf, int nbytes)
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
    conf->resultBuf = MIME_TEXT_PLAIN;
    return 1;
}


/*
 * is_tar() -- figure out whether file is a tar archive.
 *
 * Stolen (by author of file utility) from the public domain tar program: Public
 * Domain version written 26 Aug 1985 John Gilmore (ihnp4!hoptoad!gnu).
 *
 * @(#)list.c 1.18 9/23/86 Public Domain - gnu Id: mod_mime_magic.c,v 1.7
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

KMimeMagic::KMimeMagic()
{
    // Magic file detection init
    QString mimefile = locate( "mime", "magic" );
    init( mimefile );
    // Add snippets from share/config/magic/*
    QStringList snippets = KGlobal::dirs()->findAllResources( "config", "magic/*.magic", true );
    for ( QStringList::Iterator it = snippets.begin() ; it != snippets.end() ; ++it )
        if ( !mergeConfig( *it ) )
            kdWarning() << k_funcinfo << "Failed to parse " << *it << endl;
}

KMimeMagic::KMimeMagic(const QString & _configfile)
{
    init( _configfile );
}

void KMimeMagic::init( const QString& _configfile )
{
	int result;
	conf = new config_rec;

	/* set up the magic list (empty) */
	conf->magic = conf->last = NULL;
	magicResult = NULL;
	conf->followLinks = false;

        conf->utimeConf = 0L; // created on demand
	/* on the first time through we read the magic file */
	result = apprentice(_configfile);
	if (result == -1)
		return;
#ifdef MIME_MAGIC_DEBUG_TABLE
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
	kdDebug(7018) << k_funcinfo << _configfile << endl;
	int result;

	if (_configfile.isEmpty())
		return false;
	result = apprentice(_configfile);
	if (result == -1) {
		return false;
	}
#ifdef MIME_MAGIC_DEBUG_TABLE
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
#ifdef MIME_MAGIC_DEBUG_TABLE
		test_table();
#endif
		return true;
	}
	return false;
}

void
KMimeMagic::setFollowLinks( bool _enable )
{
	conf->followLinks = _enable;
}

KMimeMagicResult *
KMimeMagic::findBufferType(const QByteArray &array)
{
	unsigned char buf[HOWMANY + 1];	/* one extra for terminating '\0' */

	conf->resultBuf = QString::null;
	if ( !magicResult )
	  magicResult = new KMimeMagicResult();
	magicResult->setInvalid();
	conf->accuracy = 100;

	int nbytes = array.size();

        if (nbytes > HOWMANY)
                nbytes = HOWMANY;
        memcpy(buf, array.data(), nbytes);
        if (nbytes == 0) {
                conf->resultBuf = MIME_BINARY_ZEROSIZE;
        } else {
                buf[nbytes++] = '\0';   /* null-terminate it */
                tryit(conf, buf, nbytes);
        }
        /* if we have any results, put them in the request structure */
	magicResult->setMimeType(conf->resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(conf->accuracy);
        return magicResult;
}

static void
refineResult(KMimeMagicResult *r, const QString & _filename)
{
	QString tmp = r->mimeType();
	if (tmp.isEmpty())
		return;
	if ( tmp == "text/x-c" || tmp == "text/x-objc" )
	{
		if ( _filename.right(2) == ".h" )
			tmp += "hdr";
		else
			tmp += "src";
		r->setMimeType(tmp);
	}
	else
	if ( tmp == "text/x-c++" )
	{
		if ( _filename.endsWith(".h")
		  || _filename.endsWith(".hh")
		  || _filename.endsWith(".H")
		  || !_filename.right(4).contains('.'))
			tmp += "hdr";
		else
			tmp += "src";
		r->setMimeType(tmp);
	}
	else
	if ( tmp == "application/x-sharedlib" )
	{
		if ( _filename.find( ".so" ) == -1 ) 
		{
			tmp = "application/x-executable";
			r->setMimeType( tmp );
		}
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
#ifdef DEBUG_MIMEMAGIC
    kdDebug(7018) << "KMimeMagic::findFileType " << fn << endl;
#endif
    conf->resultBuf = QString::null;

        if ( !magicResult )
	  magicResult = new KMimeMagicResult();
	magicResult->setInvalid();
	conf->accuracy = 100;

        if ( !conf->utimeConf )
            conf->utimeConf = new KMimeMagicUtimeConf();

        /* process it based on the file contents */
        process(conf, fn );

        /* if we have any results, put them in the request structure */
        //finishResult();
	magicResult->setMimeType(conf->resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(conf->accuracy);
	refineResult(magicResult, fn);
        return magicResult;
}
