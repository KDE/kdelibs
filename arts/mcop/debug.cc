    /*

    Copyright (C) 2000-2002 Stefan Westerfeld
                       stefan@space.twc.de

	(see also below for details on the copyright of arts_strdup_printf,
	 which is taken from GLib)
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "debug.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "thread.h"

static int arts_debug_level = Arts::Debug::lInfo;
static bool arts_debug_abort = false;
static const char *arts_debug_prefix = "";
static char *messageAppName = 0;
static Arts::Mutex *arts_debug_mutex = 0;

/* routines for variable length sprintf without buffer overflow (from GLib) */
static char* arts_strdup_vprintf(const char *format, va_list args1);

namespace Arts {

static char * shell_quote(const char *s)
{
   char *result;
   char *p;
   p = result = malloc(strlen(s)*5+1);
   while(*s)
   {
     if (*s == '\'')
     {
        *p++ = '\'';
        *p++ = '"';
        *p++ = *s++;
        *p++ = '"';
        *p++ = '\'';
     }
     else
     {
        *p++ = *s++;
     }
   }
   *p = '\0';
   return result;
}

/*
 * Call the graphical application to display a message, if
 * defined. Otherwise, send to standard error. Debug messages are
 * always sent to standard error because they tend to be very verbose.
 * Note that the external application is run in the background to
 * avoid blocking the sound server.
 */
static void output_message(Debug::Level level, const char *msg) {
	char *quoted_msg;
	char *buff = 0;

	/* default to text output if no message app is defined or if it is a debug message. */
	if (messageAppName == 0 || !strcmp(messageAppName, "") || (level == Debug::lDebug))
	{
		fprintf(stderr, "%s\n", msg);
		return;
	}

        quoted_msg = shell_quote(msg);
	switch (level) {
	  case Debug::lFatal:
		  buff = arts_strdup_printf("%s -e 'Sound server fatal error:\n\n%s' &", messageAppName, quoted_msg);
		  break;
	  case Debug::lWarning:
		  buff = arts_strdup_printf("%s -w 'Sound server warning message:\n\n%s' &", messageAppName, quoted_msg);
		  break;
	  case Debug::lInfo:
		  buff = arts_strdup_printf("%s -i 'Sound server informational message:\n\n%s' &", messageAppName, quoted_msg);
		  break;
	  default:
		  break; // avoid compile warning
	}
	free(quoted_msg);
	
	if(buff != 0)
	{
	system(buff);
		free(buff);
	}
}

/*
 * Display a message using output_message. If the message is the same
 * as the previous one, just increment a count but don't display
 * it. This prevents flooding the user with duplicate warnings. If the
 * message is not the same as the previous one, then we report the
 * previously repeated message (if any) and reset the last message and
 * count.
 */
static void display_message(Debug::Level level, const char *msg) {
	static char lastMsg[1024];
	static Debug::Level lastLevel;
	static int msgCount = 0;

	if(arts_debug_mutex)
		arts_debug_mutex->lock();

	if (!strncmp(msg, lastMsg, 1024))
	{
		msgCount++;
	} else {
		if (msgCount > 0)
		{
			char *buff;
			buff = arts_strdup_printf("%s\n(The previous message was repeated %d times.)", lastMsg, msgCount);
			output_message(lastLevel, buff);
			free(buff);
		}
		strncpy(lastMsg, msg, 1024);
		lastLevel = level;
		msgCount = 0;
		output_message(level, msg);
	}

	if(arts_debug_mutex)
		arts_debug_mutex->unlock();
}

static class DebugInitFromEnv {
public:
	DebugInitFromEnv() {
		const char *env = getenv("ARTS_DEBUG");
		if(env)
		{
			if(strcmp(env,"debug") == 0)
				arts_debug_level = Debug::lDebug;
			else if(strcmp(env,"info") == 0)
				arts_debug_level = Debug::lInfo;
			else if(strcmp(env,"warning") == 0)
				arts_debug_level = Debug::lWarning;
			else if(strcmp(env,"quiet") == 0)
				arts_debug_level = Debug::lFatal;
			else
			{
				fprintf(stderr,
					"ARTS_DEBUG must be one of debug,info,warning,quiet\n");
			}
		}
		env = getenv("ARTS_DEBUG_ABORT");
		if(env)
			arts_debug_abort = true;
	}
} debugInitFromEnv;

};

void Arts::Debug::init(const char *prefix, Level level)
{
	arts_debug_level = level;
	arts_debug_prefix = prefix;
}

void Arts::Debug::fatal(const char *fmt, ...)
{
	char *buff;
    va_list ap;

    va_start(ap, fmt);
	buff = arts_strdup_vprintf(fmt, ap);
    va_end(ap);

	display_message(Debug::lFatal, buff);
	free(buff);

	if(arts_debug_abort) abort();
	exit(1);
}

void Arts::Debug::warning(const char *fmt, ...)
{
	if(lWarning >= arts_debug_level)
	{
		char *buff;
		va_list ap;

		va_start(ap, fmt);
		buff = arts_strdup_vprintf(fmt, ap);
		va_end(ap);

		display_message(Debug::lWarning, buff);
		free(buff);
	}
}

void Arts::Debug::info(const char *fmt, ...)
{
	if(lInfo >= arts_debug_level)
	{
		char *buff;
		va_list ap;

		va_start(ap, fmt);
		buff = arts_strdup_vprintf(fmt, ap);
		va_end(ap);

		display_message(Debug::lInfo, buff);
		free(buff);
	}
}

void Arts::Debug::debug(const char *fmt, ...)
{
	if(lDebug >= arts_debug_level)
	{
		char *buff;
		va_list ap;

		va_start(ap, fmt);
		buff = arts_strdup_vprintf(fmt, ap);
		va_end(ap);

		display_message(Debug::lDebug, buff);
		free(buff);
	}
}

void Arts::Debug::messageApp(const char *appName)
{
	messageAppName = (char*) realloc(messageAppName, strlen(appName)+1);
	strcpy(messageAppName, appName);
}

void Arts::Debug::initMutex()
{
	arts_return_if_fail(arts_debug_mutex == 0);

	arts_debug_mutex = new Arts::Mutex();
}

void Arts::Debug::freeMutex()
{
	arts_return_if_fail(arts_debug_mutex != 0);

	delete arts_debug_mutex;
	arts_debug_mutex = 0;
}

/*
 * For the sake of portability (snprintf is non-portable), what follows is an
 * implementation of a variant g_strdup_printf, to format debug messages of
 * an arbitary length appropriately. This is reduntant with flow/gsl/gslglib.c,
 * however, as libmcop doesn't necessarily link against gslglib.c, this is a
 * more-or-less complete copy.
 */

/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GScanner: Flexible lexical scanner for general purpose.
 * Copyright (C) 1997, 1998 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */


#include <sys/types.h>
#include <stdarg.h>
#include <string.h>

#define g_warning printf
#define g_strerror strerror

/*--- gslglib.h ---*/

#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <stdarg.h>

/* --- GLib typedefs --- */
typedef void*           gpointer;
typedef const void*     gconstpointer;
typedef char            gchar;
typedef unsigned char   guchar;
typedef signed short    gshort;
typedef unsigned short  gushort;
typedef signed int      gint;
typedef unsigned int    guint;
typedef signed long     glong;
typedef unsigned long   gulong;
typedef float           gfloat;
typedef double          gdouble;
typedef size_t          gsize;
typedef gchar           gint8;
typedef guchar          guint8;
typedef gshort          gint16;
typedef gushort         guint16;
typedef gint            gint32;
typedef guint           guint32;
typedef gint            gboolean;
typedef gint32          GTime;
#ifdef __alpha
typedef long int                gint64;
typedef unsigned long int       guint64;
#else
typedef long long int   gint64;
typedef unsigned long long int  guint64;
#endif
typedef struct _GString GString;

/* --- standard macros --- */
#ifndef ABS
#define ABS(a)          ((a) > 0 ? (a) : -(a))
#endif
#ifndef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(v,l,h)    ((v) < (l) ? (l) : (v) > (h) ? (h) : (v))
#endif
#ifndef FALSE
#define FALSE           0
#endif
#ifndef TRUE
#define TRUE            (!FALSE)
#endif
#ifndef NULL
#define NULL            ((void*) 0)
#endif

/* --- configure stuff!!! --- */
#ifdef WORDS_BIGENDIAN
#define G_BYTE_ORDER G_BIG_ENDIAN
#else
#define G_BYTE_ORDER G_LITTLE_ENDIAN
#endif

/* #define      GLIB_HAVE_STPCPY        1 */
/* Define G_VA_COPY() to do the right thing for copying va_list variables.
 * glibconfig.h may have already defined G_VA_COPY as va_copy or __va_copy.
 */
#if !defined (G_VA_COPY)
#  if defined (__GNUC__) && ( defined (__PPC__) || defined (__s390__) ) && (defined (_CALL_SYSV) || defined (_WIN32) || defined (__s390__) )
#    define G_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))
#  elif defined (G_VA_COPY_AS_ARRAY)
#    define G_VA_COPY(ap1, ap2)   g_memmove ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#    define G_VA_COPY(ap1, ap2)   ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !G_VA_COPY */

/* --- glib macros --- */
#define G_MINFLOAT      FLT_MIN
#define G_MAXFLOAT      FLT_MAX
#define G_MINDOUBLE     DBL_MIN
#define G_MAXDOUBLE     DBL_MAX
#define G_MINSHORT      SHRT_MIN
#define G_MAXSHORT      SHRT_MAX
#define G_MAXUSHORT     USHRT_MAX
#define G_MININT        INT_MIN
#define G_MAXINT        INT_MAX
#define G_MAXUINT       UINT_MAX
#define G_MINLONG       LONG_MIN
#define G_MAXLONG       LONG_MAX
#define G_MAXULONG      ULONG_MAX
#define G_USEC_PER_SEC  1000000
#define G_LITTLE_ENDIAN 1234
#define G_BIG_ENDIAN    4321

#define G_STRINGIFY(macro_or_string)    G_STRINGIFY_ARG (macro_or_string)
#define G_STRINGIFY_ARG(contents)       #contents
#if  defined __GNUC__ && !defined __cplusplus
#  define G_STRLOC      __FILE__ ":" G_STRINGIFY (__LINE__) ":" __PRETTY_FUNCTION__ "()"
#else
#  define G_STRLOC      __FILE__ ":" G_STRINGIFY (__LINE__)
#endif
       
/* subtract from biased_exponent to form base2 exponent (normal numbers) */
typedef union  _GDoubleIEEE754  GDoubleIEEE754;
typedef union  _GFloatIEEE754   GFloatIEEE754;
#define G_IEEE754_FLOAT_BIAS    (127)
#define G_IEEE754_DOUBLE_BIAS   (1023)
/* multiply with base2 exponent to get base10 exponent (nomal numbers) */
#define G_LOG_2_BASE_10         (0.30102999566398119521)
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
union _GFloatIEEE754
{
  gfloat v_float;
  struct {
    guint mantissa : 23;
    guint biased_exponent : 8;
    guint sign : 1;
  } mpn;
};
union _GDoubleIEEE754
{
  gdouble v_double;
  struct {
    guint mantissa_low : 32;
    guint mantissa_high : 20;
    guint biased_exponent : 11;
    guint sign : 1;
  } mpn;
};
#elif G_BYTE_ORDER == G_BIG_ENDIAN
union _GFloatIEEE754
{
  gfloat v_float;
  struct {
    guint sign : 1;
    guint biased_exponent : 8;
    guint mantissa : 23;
  } mpn;
};
union _GDoubleIEEE754
{
  gdouble v_double;
  struct {
    guint sign : 1;
    guint biased_exponent : 11;
    guint mantissa_high : 20;
    guint mantissa_low : 32;
  } mpn;
};
#else /* !G_LITTLE_ENDIAN && !G_BIG_ENDIAN */
#error unknown ENDIAN type
#endif /* !G_LITTLE_ENDIAN && !G_BIG_ENDIAN */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define GLIB_SIZEOF_INTMAX      (8 /* educated guess */)

typedef struct
{
  guint min_width;
  guint precision;
  gboolean alternate_format, zero_padding, adjust_left, locale_grouping;
  gboolean add_space, add_sign, possible_sign, seen_precision;
  gboolean mod_half, mod_long, mod_extra_long;
} PrintfArgSpec;


static gsize
printf_string_upper_bound (const gchar *format,
                           gboolean     may_warn,
                           va_list      args)
{
  static  gboolean honour_longs = sizeof(long) > 4 || sizeof(void*) > 4;
  gsize len = 1;

  if (!format)
    return len;

  while (*format)
    {
      register gchar c = *format++;

      if (c != '%')
        len += 1;
      else /* (c == '%') */
        {
          PrintfArgSpec spec = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
          gboolean seen_l = FALSE, conv_done = FALSE;
          gsize conv_len = 0;
          const gchar *spec_start = format;

          do
            {
              c = *format++;
              switch (c)
                {
                  GDoubleIEEE754 u_double;
                  guint v_uint;
                  gint v_int;
                  const gchar *v_string;

                  /* beware of positional parameters
                   */
                case '$':
                  if (may_warn)
                    g_warning (G_STRLOC ": unable to handle positional parameters (%%n$)");
                  len += 1024; /* try adding some safety padding */
                  break;

                  /* parse flags
                   */
                case '#':
                  spec.alternate_format = TRUE;
                  break;
                case '0':
                  spec.zero_padding = TRUE;
                  break;
                case '-':
                  spec.adjust_left = TRUE;
                  break;
                case ' ':
                  spec.add_space = TRUE;
                  break;
                case '+':
                  spec.add_sign = TRUE;
                  break;
                case '\'':
                  spec.locale_grouping = TRUE;
                  break;

                  /* parse output size specifications
                   */
                case '.':
                  spec.seen_precision = TRUE;
                  break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                  v_uint = c - '0';
                  c = *format;
                  while (c >= '0' && c <= '9')
                    {
                      format++;
                      v_uint = v_uint * 10 + c - '0';
                      c = *format;
                    }
                  if (spec.seen_precision)
                    spec.precision = MAX (spec.precision, v_uint);
                  else
                    spec.min_width = MAX (spec.min_width, v_uint);
                  break;
                case '*':
                  v_int = va_arg (args, int);
                  if (spec.seen_precision)
                    {
                      /* forget about negative precision */
                      if (v_int >= 0)
                        spec.precision = MAX (spec.precision, (unsigned)v_int);
                    }
                  else
                    {
                      if (v_int < 0)
                        {
                          v_int = - v_int;
                          spec.adjust_left = TRUE;
                        }
                      spec.min_width = MAX (spec.min_width, (unsigned)v_int);
                    }
                  break;

                  /* parse type modifiers
                   */
                case 'h':
                  spec.mod_half = TRUE;
                  break;
                case 'l':
                  if (!seen_l)
                    {
                      spec.mod_long = TRUE;
                      seen_l = TRUE;
                      break;
                    }
                  /* else, fall through */
                case 'L':
                case 'q':
                  spec.mod_long = TRUE;
                  spec.mod_extra_long = TRUE;
                  break;
                case 'z':
                case 'Z':
                  if (sizeof(size_t))
                    {
                      spec.mod_long = TRUE;
                      spec.mod_extra_long = TRUE;
                    }
                  break;
                case 't':
                  if (sizeof(ptrdiff_t) > 4)
                    {
                      spec.mod_long = TRUE;
                      spec.mod_extra_long = TRUE;
                    }
                  break;
                case 'j':
                  if (GLIB_SIZEOF_INTMAX > 4)
                    {
                      spec.mod_long = TRUE;
                      spec.mod_extra_long = TRUE;
                    }
                  break;

                  /* parse output conversions
                   */
                case '%':
                  conv_len += 1;
                  break;
                case 'O':
                case 'D':
                case 'I':
                case 'U':
                  /* some C libraries feature long variants for these as well? */
                  spec.mod_long = TRUE;
                  /* fall through */
                case 'o':
                  conv_len += 2;
                  /* fall through */
                case 'd':
                case 'i':
                  conv_len += 1; /* sign */
                  /* fall through */
                case 'u':
                  conv_len += 4;
                  /* fall through */
                case 'x':
                case 'X':
                  spec.possible_sign = TRUE;
                  conv_len += 10;
                  if (spec.mod_long && honour_longs)
                    conv_len *= 2;
                  if (spec.mod_extra_long)
                    conv_len *= 2;
                  if (spec.mod_extra_long)
                    {
                      (void) va_arg (args, gint64);
                    }
                  else if (spec.mod_long)
                    (void) va_arg (args, long);
                  else
                    (void) va_arg (args, int);
                  break;
                case 'A':
                case 'a':
                  /*          0x */
                  conv_len += 2;
                  /* fall through */
                case 'g':
                case 'G':
                case 'e':
                case 'E':
                case 'f':
                  spec.possible_sign = TRUE;
                  /*          n   .   dddddddddddddddddddddddd   E   +-  eeee */
                  conv_len += 1 + 1 + MAX (24, spec.precision) + 1 + 1 + 4;
                  if (may_warn && spec.mod_extra_long)
                    g_warning (G_STRLOC ": unable to handle long double, collecting double only");
#ifdef HAVE_LONG_DOUBLE
#error need to implement special handling for long double
#endif
                  u_double.v_double = va_arg (args, double);
                  /* %f can expand up to all significant digits before '.' (308) */
                  if (c == 'f' &&
                      u_double.mpn.biased_exponent > 0 && u_double.mpn.biased_exponent < 2047)
                    {
                      gint exp = u_double.mpn.biased_exponent;

                      exp -= G_IEEE754_DOUBLE_BIAS;
                      exp = (gint)(exp * G_LOG_2_BASE_10 + 1);
                      conv_len += ABS (exp);    /* exp can be <0 */
                    }
                  /* some printf() implementations require extra padding for rounding */
                  conv_len += 2;
                  /* we can't really handle locale specific grouping here */
                  if (spec.locale_grouping)
                    conv_len *= 2;
                  break;
                case 'C':
                  spec.mod_long = TRUE;
                  /* fall through */
                case 'c':
                  conv_len += spec.mod_long ? MB_LEN_MAX : 1;
                  (void) va_arg (args, int);
                  break;
                case 'S':
                  spec.mod_long = TRUE;
                  /* fall through */
                case 's':
                  v_string = va_arg (args, char*);
                  if (!v_string)
                    conv_len += 8; /* hold "(null)" */
                  else if (spec.seen_precision)
                    conv_len += spec.precision;
                  else
                    conv_len += strlen (v_string);
                  conv_done = TRUE;
                  if (spec.mod_long)
                    {
                      if (may_warn)
                        g_warning (G_STRLOC": unable to handle wide char strings");
                      len += 1024; /* try adding some safety padding */
                    }
                  break;
                case 'P': /* do we actually need this? */
                  /* fall through */
                case 'p':
                  spec.alternate_format = TRUE;
                  conv_len += 10;
                  if (honour_longs)
                    conv_len *= 2;
                  /* fall through */
                case 'n':
                  conv_done = TRUE;
                  (void) va_arg (args, void*);
                  break;
                case 'm':
                  /* there's not much we can do to be clever */
                  v_string = g_strerror (errno);
                  v_uint = v_string ? strlen (v_string) : 0;
                  conv_len += MAX (256, v_uint);
                  break;

                  /* handle invalid cases
                   */
                case '\000':
                  /* no conversion specification, bad bad */
                  conv_len += format - spec_start;
                  break;
                default:
                  if (may_warn)
                    g_warning (G_STRLOC": unable to handle `%c' while parsing format",
                               c);
                  break;
                }
              conv_done |= conv_len > 0;
            }
          while (!conv_done);
          /* handle width specifications */
          conv_len = MAX (conv_len, MAX (spec.precision, spec.min_width));
          /* handle flags */
          conv_len += spec.alternate_format ? 2 : 0;
          conv_len += (spec.add_space || spec.add_sign || spec.possible_sign);
          /* finally done */
          len += conv_len;
        } /* else (c == '%') */
    } /* while (*format) */

  return len;
}

static char*
arts_strdup_vprintf (const char *format, va_list args1)
{
  gchar *buffer;
  va_list args2;

  G_VA_COPY (args2, args1);

  buffer = (gchar *)malloc (printf_string_upper_bound (format, TRUE, args1));

  vsprintf (buffer, format, args2);
  va_end (args2);

  return buffer;
}

char*
arts_strdup_printf (const char *format, ...)
{
  gchar *buffer;
  va_list args;

  va_start (args, format);
  buffer = arts_strdup_vprintf (format, args);
  va_end (args);

  return buffer;
}
