/* Evil evil evil hack to get OSS apps to cooperate with artsd
 * This is based on the original esddsp, which esd uses to do the same.
 *
 * Copyright (C) 1998 Manish Singh <yosh@gimp.org>
 * Copyright (C) 2000 Stefan Westerfeld <stefan@space.twc.de> (aRts port)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <artsc.h>
#include <dlfcn.h>


/*
 * NOTE:
 *
 * To truely support non-blocking I/O, there is some stuff missing. Namely,
 * select should be trapped and redirected, as well as the means for making
 * a stream non-blocking and so on. Maybe poll, too.
 *
 * Currently, only apps that are doing blocking I/O will probably work right.
 */

/**
 * the stream status: sndfd is -1 when unused, otherwise it is a useless fd
 * which points to /dev/null, to ensure compatibility with more weird
 * operations on streams - arts_init has been run (and must be freed again)
 * if sndfd != -1
 *
 * settings contains what has already been set (speed, bits, channels), and
 * is 7 when all of these are true
 *
 * stream contains an aRts stream or 0
 */
static int sndfd = -1;
static int settings;
static arts_stream_t stream = 0;

#if !defined(HAVE_IOCTL_INT_INT_DOTS) && !defined(HAVE_IOCTL_INT_ULONG_DOTS)
#error "unknown ioctl type (check config.h, adapt configure test)..."
#endif

/*
 * original C library functions
 */
typedef int (*orig_open_ptr)(const char *pathname, int flags, ...);
typedef int (*orig_close_ptr)(int fd);
#ifdef HAVE_IOCTL_INT_INT_DOTS
typedef int (*orig_ioctl_ptr)(int fd, int request, ...);
#else /* HAVE_IOCTL_INT_ULONG_DOTS */
typedef int (*orig_ioctl_ptr)(int fd, unsigned long request, ...);
#endif
typedef ssize_t (*orig_write_ptr)(int fd, const void *buf, size_t count);

static orig_open_ptr orig_open;
static orig_close_ptr orig_close;
static orig_ioctl_ptr orig_ioctl;
static orig_write_ptr orig_write;

static int artsdsp_debug = 0;
static int artsdsp_init = 0;

#define CHECK_INIT() if(!artsdsp_init) artsdsp_doinit();

/*
 * Initialization - maybe this should be either be a startup only called
 * routine, or use pthread locks to prevent strange effects in multithreaded
 * use (however it seems highly unlikely that an application would create
 * multiple threads before even using one of redirected the system functions
 * once).
 */ 

static void artsdsp_doinit()
{
	const char *verbose;
	artsdsp_init = 1;

	/* debugging? */
	verbose = getenv("ARTSDSP_VERBOSE");
	artsdsp_debug = verbose && !strcmp(verbose,"1");

	/* resolve original symbols */
	orig_open = (orig_open_ptr)dlsym(RTLD_NEXT,"open");
	orig_close = (orig_close_ptr)dlsym(RTLD_NEXT,"close");
	orig_write = (orig_write_ptr)dlsym(RTLD_NEXT,"write");
	orig_ioctl = (orig_ioctl_ptr)dlsym(RTLD_NEXT,"ioctl");
}

static void artsdspdebug(const char *fmt,...)
{
	CHECK_INIT();

	if(artsdsp_debug)
	{
    	va_list ap;
    	va_start(ap, fmt);
    	(void) vfprintf(stderr, fmt, ap);
    	va_end(ap);
	}
}

int open (const char *pathname, int flags, ...)
{
  va_list args;
  mode_t mode = 0;

  CHECK_INIT();

  /*
   * After the documentation, va_arg is not safe if there is no argument to
   * get "random errors will occur", so only get it in case O_CREAT is set,
   * and hope that passing 0 to the orig_open function in all other cases
   * will work.
   */
  va_start(args,flags);
  if(flags & O_CREAT) mode = va_arg(args, mode_t);
  va_end(args);

  if (strcmp(pathname,"/dev/dsp"))    /* original open for anything but sound */
    return orig_open (pathname, flags, mode);

  settings = 0;
  stream = 0;

  artsdspdebug ("aRts: hijacking /dev/dsp open...\n");

  sndfd = orig_open("/dev/null",flags,mode);
  if(sndfd >= 0)
  {
    int rc = arts_init();
    if(rc < 0)
    {
      artsdspdebug("error on aRts init: %s\n", arts_error_text(rc));
      orig_close(sndfd);
      return -1;
    }
  }

  /* success */
  return sndfd;
}

#ifdef HAVE_IOCTL_INT_INT_DOTS
int ioctl (int fd, int request, ...)
#else /* HAVE_IOCTL_INT_ULONG_DOTS */
int ioctl (int fd, unsigned long request, ...)
#endif
{
  static int channels;
  static int bits;
  static int speed;

  /*
   * FreeBSD needs ioctl with varargs. However I have no idea how to "forward"
   * the variable args ioctl to the orig_ioctl routine. So I expect the ioctl
   * to have exactly one pointer-like parameter and forward that, hoping that
   * it works
   */
  va_list args;
  void *argp;
  va_start(args,request);
  argp = va_arg(args, void *);
  va_end(args);

  CHECK_INIT();

  if (fd != sndfd)
    return orig_ioctl (fd, request, argp);
  else if (sndfd != -1)
    {
      int *arg = (int *) argp;
      artsdspdebug("aRts: hijacking /dev/dsp ioctl (%d : %x - %p)\n",
			  fd, request, argp);

      switch (request)
        {
        case SNDCTL_DSP_SETFMT:
          bits = (*arg & 0x30) ? 16 : 8;
          settings |= 1;
          break;

        case SNDCTL_DSP_SPEED:
          speed = *arg;
          settings |= 2;
          break;

        case SNDCTL_DSP_STEREO:
          channels = (*arg)?2:1;
          settings |= 4;
          break;

        case SNDCTL_DSP_GETBLKSIZE:
          *arg = stream?arts_stream_get(stream,ARTS_P_PACKET_SIZE):16384;
          break;

        case SNDCTL_DSP_GETFMTS:
          *arg = 0x38;
          break;

        case SNDCTL_DSP_GETCAPS:
          *arg = 0;
          break;

        default:
          artsdspdebug("aRts: unhandled /dev/dsp ioctl (%x - %p)\n",request, argp);
          break;
        }

      if (settings == 7 && !stream)
        {
		  const char *name = getenv("ARTSDSP_NAME");

          artsdspdebug ("aRts: creating stream...\n");
	      stream = arts_play_stream(speed,bits,channels,name?name:"artsdsp");
        }

      return 0;
    }

  return 0;
}

int close(int fd)
{
  CHECK_INIT();

  if (fd != sndfd)
    return orig_close (fd);
  else if (sndfd != -1)
    {
      artsdspdebug ("aRts: /dev/dsp close...\n");
      if(stream)
      {
        arts_close_stream(stream);
        stream = 0;
      }

      arts_free();

      orig_close(sndfd);
      sndfd = -1;
    }
  return 0;
}

ssize_t write (int fd, const void *buf, size_t count)
{
  CHECK_INIT();

  if(fd != sndfd)
    return orig_write(fd,buf,count);
  else if(sndfd != -1)
  {
    artsdspdebug ("aRts: /dev/dsp write...\n");
	if(stream != 0)
	{
      return arts_write(stream,buf,count);
	}
  }
  return 0;
}
#endif
