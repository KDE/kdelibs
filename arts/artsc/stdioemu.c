    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

/*
 * This source only exists because some very special programs think that
 * it is a very special idea to access /dev/dsp by the means of stdio, so
 * we need to fake FILE* access for artsdsp as well.
 *
 * To do so, it relies on glibc internals, so that it will probably not work
 * on other systems - but then again, it might not be necessary on other
 * systems, when fopen properly calls open, it might as well work unchanged.
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <libio.h>

struct fd_cookie {
	int fd;
};

static ssize_t fdc_read (void *cookie, char *buffer, size_t size)
{
	struct fd_cookie *fdc = (struct fd_cookie *)cookie;
	return read(fdc->fd, buffer, size);
}

static ssize_t fdc_write (void *cookie, const char *buffer, size_t size)
{
	struct fd_cookie *fdc = (struct fd_cookie *)cookie;
	return write(fdc->fd, buffer, size);
}

static int fdc_seek (void *cookie, off64_t *position, int whence)
{
	return -1;
}

static int fdc_clean (void *cookie)
{
	struct fd_cookie *fdc = (struct fd_cookie *)cookie;
	int result = close(fdc->fd);
	free(cookie);
	return result;
}

static FILE *fake_fopen(const char *path, const char *mode)
{
	cookie_io_functions_t fns = { fdc_read, fdc_write, fdc_seek, fdc_clean };
	struct fd_cookie *fdc =
		(struct fd_cookie *)malloc(sizeof(struct fd_cookie));
	const char *mptr;
	int open_mode = 0;
	FILE *result = 0;

	for(mptr = mode; *mptr; mptr++)
	{
		if(*mptr == 'r') open_mode |= 1; /* 1 = read */
		if(*mptr == 'w') open_mode |= 2; /* 2 = write */
		if(*mptr == '+') open_mode |= 3; /* 3 = readwrite */
		if(*mptr == 'a') open_mode |= 2; /* append -> write */
  	}
  	if(open_mode == 1) fdc->fd = open(path,O_RDONLY,0666);
  	if(open_mode == 2) fdc->fd = open(path,O_WRONLY,0666);
  	if(open_mode == 3) fdc->fd = open(path,O_RDWR,0666);

	if(open_mode && fdc->fd > 0)
	{
		result = fopencookie (fdc,"w", fns);
		result->_fileno = fdc->fd;		/* ugly patchy slimy kludgy hack */
	}
	return result;
}
