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

#include "artsc.h"
#include "ltdl.h"
#include <assert.h>

static struct arts_backend {
	int available;
	int refcnt;
	lt_dlhandle handle;

	int (*init)();
	void (*free)();
	arts_stream_t (*play_stream)(int,int,int,char*);
	arts_stream_t (*record_stream)(int,int,int,char*);
	void (*close_stream)(arts_stream_t);
	int (*read)(arts_stream_t,void*,int);
	int (*write)(arts_stream_t,void *,int);
} backend = { 0,0,0,0,0,0,0,0,0,0 };

static void arts_backend_ref()
{
	if(backend.refcnt == 0)
	{
    	lt_dlinit();
		backend.handle = lt_dlopen(ARTSC_BACKEND);

		if(backend.handle)
		{
			backend.init =
				lt_dlsym(backend.handle, "arts_backend_init");
			backend.free =
				lt_dlsym(backend.handle, "arts_backend_free");
			backend.play_stream =
				lt_dlsym(backend.handle, "arts_backend_play_stream");
			backend.record_stream =
				lt_dlsym(backend.handle, "arts_backend_record_stream");
			backend.close_stream =
				lt_dlsym(backend.handle, "arts_backend_close_stream");
			backend.write =
				lt_dlsym(backend.handle, "arts_backend_write");
			backend.read =
				lt_dlsym(backend.handle, "arts_backend_read");
		}

		if(backend.handle && backend.init && backend.free && backend.play_stream
			&& backend.record_stream && backend.close_stream && backend.write
			&& backend.read)
			backend.available = 1;
		else
			backend.available = 0;
	}
	backend.refcnt++;
}

static void arts_backend_release()
{
	assert(backend.refcnt > 0);
	backend.refcnt--;
	if(backend.refcnt == 0)
	{
		if(backend.available)
		{
			backend.available = 0;

			if(backend.handle) lt_dlclose(backend.handle);
		}
		lt_dlexit();
	}
}

int arts_init()
{
	int rc = ARTS_E_NOBACKEND;

	arts_backend_ref();
	if(backend.available) rc = backend.init();

	/* init failed: the user may not call other arts_xxx functions now */
	if(rc < 0) arts_backend_release();

	return rc;
}

void arts_free()
{
	if(backend.available)
	{
		backend.free();
		arts_backend_release();
	}
}

arts_stream_t arts_play_stream(int rate, int bits, int channels, char *name)
{
	arts_stream_t rc = 0;

	if(backend.available) rc = backend.play_stream(rate,bits,channels,name);
	return rc;
}

arts_stream_t arts_record_stream(int rate, int bits, int channels, char *name)
{
	arts_stream_t rc = 0;

	if(backend.available) rc = backend.record_stream(rate,bits,channels,name);
	return rc;
}

void arts_close_stream(arts_stream_t stream)
{
	if(backend.available) backend.close_stream(stream);
}

int arts_read(arts_stream_t stream, void *buffer, int count)
{
	int rc = ARTS_E_NOBACKEND;

	if(backend.available) rc = backend.read(stream,buffer,count);
	return rc;
}

int arts_write(arts_stream_t stream, void *buffer, int count)
{
	int rc = ARTS_E_NOBACKEND;

	if(backend.available) rc = backend.write(stream,buffer,count);
	return rc;
}

const char *arts_error_text(int errorcode)
{
	switch(errorcode) {
		case ARTS_E_NOSERVER:
			return "can't connect to aRts soundserver";
		case ARTS_E_NOBACKEND:
			return "loading the aRts backend \""
						ARTSC_BACKEND "\" failed";
		case ARTS_E_NOIMPL:
			return "this aRts function is not yet implemented";
		case ARTS_E_NOINIT:
			return "need to use arts_init() before using other functions";
		case ARTS_E_NOSTREAM:
			return "you passed no valid aRts stream to a function";
	}
	return "unknown arts error happend";
}
