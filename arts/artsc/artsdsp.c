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

#define _GNU_SOURCE 1

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
/* #include <sys/mman.h> */
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
 * operations on streams
 *
 * settings contains what has already been set (speed, bits, channels), and
 * is 7 when all of these are true
 *
 * stream contains an aRts stream or 0
 */
static int sndfd = -1;
static int settings;
static int arts_init_done = 0;
static arts_stream_t stream = 0;
static int frags;

#if defined(HAVE_IOCTL_INT_INT_DOTS)
typedef int ioctl_request_t;
#elif defined(HAVE_IOCTL_INT_ULONG_DOTS)
typedef unsigned long ioctl_request_t;
#elif defined(HAVE_IOCTL_INT_ULONGINT_DOTS)
typedef unsigned long int ioctl_request_t;
#else
#error "unknown ioctl type (check config.h, adapt configure test)..."
#endif

/*
 * memory mapping emulation
 */
static int mmapemu = 0;
static count_info mmapemu_ocount;
static char *mmapemu_obuffer = 0;
static int mmapemu_osize;

/*
 * original C library functions
 */
typedef int (*orig_open_ptr)(const char *pathname, int flags, ...);
typedef int (*orig_close_ptr)(int fd);
typedef int (*orig_ioctl_ptr)(int fd, ioctl_request_t request, ...);
typedef ssize_t (*orig_write_ptr)(int fd, const void *buf, size_t count);
typedef void* (*orig_mmap_ptr)(void *start, size_t length, int prot,
                                 int flags, int fd, off_t offset);
typedef int (*orig_munmap_ptr)(void *start, size_t length);
typedef FILE* (*orig_fopen_ptr)(const char *path, const char *mode);
typedef int (*orig_access_ptr)(const char *pathname, int mode);   

static orig_open_ptr orig_open;
static orig_close_ptr orig_close;
static orig_ioctl_ptr orig_ioctl;
static orig_write_ptr orig_write;
static orig_mmap_ptr orig_mmap;
static orig_munmap_ptr orig_munmap;
static orig_fopen_ptr orig_fopen;
static orig_access_ptr orig_access;

static int artsdsp_debug = 0;
static int artsdsp_init = 0;

void *mmap(void  *start,  size_t length, int prot, int flags,
                     int fd, off_t offset);
int munmap(void *start, size_t length);
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
	const char *env;
	artsdsp_init = 1;

	/* debugging? */
	env = getenv("ARTSDSP_VERBOSE");
	artsdsp_debug = env && !strcmp(env,"1");

	/* mmap emulation? */
	env = getenv("ARTSDSP_MMAP");
	mmapemu = env && !strcmp(env,"1");

	/* resolve original symbols */
	orig_open = (orig_open_ptr)dlsym(RTLD_NEXT,"open");
	orig_close = (orig_close_ptr)dlsym(RTLD_NEXT,"close");
	orig_write = (orig_write_ptr)dlsym(RTLD_NEXT,"write");
	orig_ioctl = (orig_ioctl_ptr)dlsym(RTLD_NEXT,"ioctl");
	orig_mmap = (orig_mmap_ptr)dlsym(RTLD_NEXT,"mmap");
	orig_munmap = (orig_munmap_ptr)dlsym(RTLD_NEXT,"munmap");
	orig_fopen = (orig_fopen_ptr)dlsym(RTLD_NEXT,"fopen");
	orig_access = (orig_access_ptr)dlsym(RTLD_NEXT,"access");
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

static void mmapemu_flush()
{
  int space = arts_stream_get(stream, ARTS_P_BUFFER_SPACE);
  artsdspdebug("space = %d\n",space);
  while(space >= 4096)
  {
    arts_write(stream,&mmapemu_obuffer[mmapemu_ocount.ptr],4096);
    space -= 4096;
    mmapemu_ocount.ptr += 4096;
    mmapemu_ocount.ptr %= mmapemu_osize;
    mmapemu_ocount.blocks++;
    mmapemu_ocount.bytes += 4096;
  }
}

/* returns 1 if the filename points to a sound device */
static int is_sound_device(const char *pathname)
{
  if(strcmp(pathname,"/dev/dsp") == 0) return 1;
  if(strcmp(pathname,"/dev/sound/dsp") == 0) return 1;
  return 0;
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

  if (!is_sound_device(pathname))    /* original open for anything but sound */
    return orig_open (pathname, flags, mode);

  settings = 0;
  frags = 0;
  stream = 0;

  artsdspdebug ("aRts: hijacking /dev/dsp open...\n");

  sndfd = orig_open("/dev/null",flags,mode);
  if(sndfd >= 0)
  {
    if(!arts_init_done) {
	  int rc = arts_init();
	  if(rc < 0) {
	    artsdspdebug("error on aRts init: %s\n", arts_error_text(rc));
	    orig_close(sndfd);
	    sndfd = -1;
	    return orig_open (pathname, flags, mode);
	  }
	  else arts_init_done = 1;
    }
  }

  /* success */
  return sndfd;
}

int ioctl (int fd, ioctl_request_t request, ...)
{
  static int channels;
  static int bits;
  static int speed;
  int space, size, latency, odelay;

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

  if (fd != sndfd )
    return orig_ioctl (fd, request, argp);
  else if (sndfd != -1)
    {
      int *arg = (int *) argp;
      artsdspdebug("aRts: hijacking /dev/dsp ioctl (%d : %x - %p)\n",
              fd, request, argp);

      switch (request)
        {
        struct audio_buf_info *audiop;
#ifdef SNDCTL_DSP_RESET
		case SNDCTL_DSP_RESET:              /* _SIO  ('P', 0) */
		  artsdspdebug("aRts: SNDCTL_DSP_RESET unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SYNC
		case SNDCTL_DSP_SYNC:               /* _SIO  ('P', 1) */
		  artsdspdebug("aRts: SNDCTL_DSP_SYNC unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SPEED
        case SNDCTL_DSP_SPEED:				/* _SIOWR('P', 2, int) */
          speed = *arg;
          settings |= 2;
          break;
#endif

#ifdef SNDCTL_DSP_STEREO
        case SNDCTL_DSP_STEREO:				/* _SIOWR('P', 3, int) */
          channels = (*arg)?2:1;
          settings |= 4;
          break;
#endif

#ifdef SNDCTL_DSP_GETBLKSIZE
        case SNDCTL_DSP_GETBLKSIZE:			/* _SIOWR('P', 4, int) */
          if(mmapemu)
            *arg = 4096;
          else if(stream)
            *arg = arts_stream_get(stream,ARTS_P_PACKET_SIZE);
          else
          {
            int fragSize = frags & 0x7fff;
            if(fragSize > 1 && fragSize < 16)
              *arg = (1 << fragSize);
            else
              *arg = 16384; /* no idea ;-) */
          }
          break;
#endif

#ifdef SNDCTL_DSP_SETFMT
        case SNDCTL_DSP_SETFMT:				/* _SIOWR('P',5, int) */
          bits = (*arg & 0x30) ? 16 : 8;
          settings |= 1;
          break;
#endif

#ifdef SNDCTL_DSP_CHANNELS
        case SNDCTL_DSP_CHANNELS:			/*  _SIOWR('P', 6, int) */
          channels = (*arg);
          settings |= 4;
          break;
#endif

#ifdef SOUND_PCM_WRITE_FILTER
		case SOUND_PCM_WRITE_FILTER:        /* _SIOWR('P', 7, int) */
		  artsdspdebug("aRts: SNDCTL_DSP_WRITE_FILTER(%d) unsupported\n",*arg);
		  break;
#endif

#ifdef SNDCTL_DSP_POST
		case SNDCTL_DSP_POST:               /* _SIO  ('P', 8) */
		  artsdspdebug("aRts: SNDCTL_DSP_POST unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SUBDIVIDE
		case SNDCTL_DSP_SUBDIVIDE:          /* _SIOWR('P', 9, int) */
		  artsdspdebug("aRts: SNDCTL_DSP_SUBDIVIDE(%d) unsupported\n",*arg);
		  break;
#endif

#ifdef SNDCTL_DSP_SETFRAGMENT
        case SNDCTL_DSP_SETFRAGMENT:        /* _SIOWR('P',10, int) */
          artsdspdebug("aRts: SNDCTL_DSP_SETFRAGMENT(%x) partially supported\n",
                          *arg);
          frags = *arg;
          break;
#endif

#ifdef SNDCTL_DSP_GETFMTS
        case SNDCTL_DSP_GETFMTS:            /* _SIOR ('P',11, int) */
          *arg = 8 | 16;
          break;
#endif

#if defined(SNDCTL_DSP_GETOSPACE) && defined(SNDCTL_DSP_GETISPACE)
        case SNDCTL_DSP_GETOSPACE:          /* _SIOR ('P',12, audio_buf_info) */
        case SNDCTL_DSP_GETISPACE:          /* _SIOR ('P',13, audio_buf_info) */
          audiop = argp;
		  if(mmapemu)
		  {
			audiop->fragstotal = 16;
			audiop->fragsize = 4096;
			audiop->bytes = 0;              /* FIXME: is this right? */
			audiop->fragments = 0;
		  }
		  else
		  {
            audiop->fragstotal =
              stream?arts_stream_get(stream, ARTS_P_PACKET_COUNT):10;
            audiop->fragsize =
              stream?arts_stream_get(stream, ARTS_P_PACKET_SIZE):16384;
            audiop->bytes =
              stream?arts_stream_get(stream, ARTS_P_BUFFER_SPACE):16384;
            audiop->fragments = audiop->bytes / audiop->fragsize;
		  }
          break;
#endif

#ifdef SNDCTL_DSP_NONBLOCK
		case SNDCTL_DSP_NONBLOCK:           /* _SIO  ('P',14) */
		  artsdspdebug("aRts: SNDCTL_DSP_NONBLOCK unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_GETCAPS
        case SNDCTL_DSP_GETCAPS:			/* _SIOR ('P',15, int) */
		  if(mmapemu)
			*arg = DSP_CAP_MMAP | DSP_CAP_TRIGGER | DSP_CAP_REALTIME;
		  else
            *arg = 0;
          break;
#endif

#ifdef SNDCTL_DSP_GETTRIGGER
		case SNDCTL_DSP_GETTRIGGER:         /* _SIOR ('P',16, int) */
		  artsdspdebug("aRts: SNDCTL_DSP_GETTRIGGER unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SETTRIGGER
		case SNDCTL_DSP_SETTRIGGER:         /* _SIOW ('P',16, int) */
		  artsdspdebug("aRts: SNDCTL_DSP_SETTRIGGER(%d) unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_GETIPTR
        case SNDCTL_DSP_GETIPTR:			/* _SIOR ('P',17, count_info) */
		  artsdspdebug("aRts: SNDCTL_DSP_GETIPTR unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_GETOPTR
        case SNDCTL_DSP_GETOPTR:			/* _SIOR ('P',18, count_info) */
		  if(mmapemu)
		  {
			mmapemu_flush();
            *((count_info *)arg) = mmapemu_ocount;
            mmapemu_ocount.blocks = 0;     
		  }
		  else
		  {
			artsdspdebug("aRts: SNDCTL_DSP_GETOPTR unsupported\n");
		  }
		  break;
#endif

#ifdef SNDCTL_DSP_MAPINBUF
		case SNDCTL_DSP_MAPINBUF:           /* _SIOR ('P', 19, buffmem_desc) */
		  artsdspdebug("aRts: SNDCTL_DSP_MAPINBUF unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_MAPOUTBUF
		case SNDCTL_DSP_MAPOUTBUF:          /* _SIOR ('P', 20, buffmem_desc) */
		  artsdspdebug("aRts: SNDCTL_DSP_MAPOUTBUF unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SETSYNCRO
		case SNDCTL_DSP_SETSYNCRO:          /* _SIO  ('P', 21) */
		  artsdspdebug("aRts: SNDCTL_DSP_SETSYNCHRO unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_SETDUPLEX
		case SNDCTL_DSP_SETDUPLEX:          /* _SIO  ('P', 22) */
		  artsdspdebug("aRts: SNDCTL_DSP_SETDUPLEX unsupported\n");
		  break;
#endif

#ifdef SNDCTL_DSP_GETODELAY
		case SNDCTL_DSP_GETODELAY:          /* _SIOR ('P', 23, int) */
		  space = arts_stream_get(stream, ARTS_P_BUFFER_SPACE);
		  size = arts_stream_get(stream, ARTS_P_BUFFER_SIZE);
		  latency = arts_stream_get(stream, ARTS_P_SERVER_LATENCY);
		  odelay = size - space + (latency * speed * bits * channels) / 8000;
		  artsdspdebug("aRts: SNDCTL_DSP_GETODELAY returning %d\n", odelay);
		  *arg = odelay;
		  break;
#endif

        default:
          artsdspdebug("aRts: unhandled /dev/dsp ioctl (%x - %p)\n",request, argp);
          break;
        }

      if (settings == 7 && !stream)
        {
          const char *name = getenv("ARTSDSP_NAME");
          int fragSize = (frags & 0x7fff);
          int fragCount = (frags & 0x7fff0000) >> 16;

          artsdspdebug ("aRts: creating stream...\n");
          stream = arts_play_stream(speed,bits,channels,name?name:"artsdsp");

          if(fragSize > 1 && fragSize < 16)
          {
            /*
             * if fragment settings are way too large (unrealistic), we
             * will assume that the user didn't mean it, and let the C API
             * choose a convenient number >= 3
             */
            if(fragCount < 2 || fragCount > 8192
            || (fragCount * (1 << fragSize)) > 128*1024)
            {
              frags = 0x00030000+fragSize;
            }

            arts_stream_set(stream,ARTS_P_PACKET_SETTINGS,frags);
          }
		  if(mmapemu)
		  {
		    arts_stream_set(stream,ARTS_P_PACKET_SETTINGS,0x0002000c);
		    mmapemu_ocount.ptr=mmapemu_ocount.blocks=mmapemu_ocount.bytes=0;
			artsdspdebug("aRts: total latency = %dms, buffer latency = %dms\n",
			  arts_stream_get(stream,ARTS_P_TOTAL_LATENCY),
              arts_stream_get(stream, ARTS_P_BUFFER_TIME));
		  }
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
	  if(mmapemu && mmapemu_obuffer)
	  {
		free(mmapemu_obuffer);
		mmapemu_obuffer = 0;
	  }

	  /*
	   * there are problems with library unloading in conjunction with X11,
	   * (Arts::X11GlobalComm), so we don't unload stuff again here
	   */

      /* arts_free(); */

      orig_close(sndfd);
      sndfd = -1;
    }
  return 0;
}

int access(const char *pathname, int mode)
{
  CHECK_INIT();

  if (!is_sound_device(pathname))    /* original open for anything but sound */
    return orig_access (pathname, mode);
  else
    artsdspdebug ("aRts: /dev/dsp access...\n");
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

void *mmap(void  *start,  size_t length, int prot, int flags,
             int fd, off_t offset)
{
  CHECK_INIT();

  if(fd != sndfd || sndfd == -1)
    return orig_mmap(start,length,prot,flags,fd,offset);
  else
  {
    artsdspdebug ("aRts: mmap - start = %x, length = %d, prot = %d\n",
                                                          start, length, prot);
    artsdspdebug ("      flags = %d, fd = %d, offset = %d\n",flags, fd,offset);

	if(mmapemu)
	{
      mmapemu_osize = length;
      mmapemu_obuffer = malloc(length);
	  mmapemu_ocount.ptr = mmapemu_ocount.blocks = mmapemu_ocount.bytes = 0;
      return mmapemu_obuffer;
	}
	else artsdspdebug ("aRts: /dev/dsp mmap (unsupported, try -m option)...\n");
  }
  return (void *)-1;
}
        
int munmap(void *start, size_t length)
{
  CHECK_INIT();

  if(start != mmapemu_obuffer || mmapemu_obuffer == 0)
    return orig_munmap(start,length);

  artsdspdebug ("aRts: /dev/dsp munmap...\n");
  mmapemu_obuffer = 0;
  free(start);

  return 0;
}

#ifdef HAVE_ARTSDSP_STDIO_EMU

/*
 * use #include rather than linking, because we want to keep everything
 * static inside artsdsp to be sure that we don't override application symbols
 */
#include "stdioemu.c"

FILE* fopen(const char *path, const char *mode)
{
  CHECK_INIT();

  if (!is_sound_device(path))    /* original open for anything but sound */
    return orig_fopen (path, mode);

  artsdspdebug ("aRts: hijacking /dev/dsp fopen...\n");

  return fake_fopen(path, mode);
}
#endif

#endif
/*
 * vim:ts=4
 */
