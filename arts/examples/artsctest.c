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
 * This is another artscat written with aRts C API. It reads data from
 * stdin, and plays it via the aRts soundserver.
 * 
 * Compile programs using the aRts C API with
 * 
 *    cc -o artsctest artsctest.c `artsc-config --cflags` `artsc-config --libs`
 *
 * If you are using a makefile, it could look like this:
 *
 *    CFLAGS=`artsc-config --cflags`
 *    LDFLAGS=`artsc-config --libs`
 *
 *    artsctest: artsctest.c
 */

#include <artsc.h>
#include <stdio.h>

int main()
{
	arts_stream_t stream;
	char buffer[8192];
	int bytes;
	int errorcode;

	errorcode = arts_init();
	if(errorcode < 0)
	{
		fprintf(stderr,"arts_init error: %s\n", arts_error_text(errorcode));
		return 1;
	}

	stream = arts_play_stream(44100,16,2,"artsctest");

	while((bytes = fread(buffer,1,8192,stdin)) > 0)
	{
		errorcode = arts_write(stream,buffer,bytes);
		if(errorcode < 0)
		{
			fprintf(stderr,"arts_write error: %s\n",arts_error_text(errorcode));
			return 1;
		}
	}

	arts_close_stream(stream);
	arts_free();

	return 0;
}
