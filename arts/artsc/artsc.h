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

#ifndef ARTSC_H
#define ARTSC_H

/**
 * @libdoc aRts plain C API
 *
 * The aRts plain C API aims at easily writing/porting plain C apps to the
 * arts sound server. What is provided is streaming functionality, in a
 * blocking way. So for most apps, you simply remove the few system calls
 * that deal with your audio device, and replace them with the appropriate
 * arts calls.
 */

/**
 * the type of streams (simply treat these as black boxes)
 */
typedef void *arts_stream_t;

/* error codes */

#define ARTS_E_NOSERVER     ( -1 )
#define ARTS_E_NOBACKEND    ( -2 )
#define ARTS_E_NOSTREAM		( -3 )
#define ARTS_E_NOINIT		( -4 )
#define ARTS_E_NOIMPL		( -5 )

/**
 * initializes the aRts C API, and connects to the sound server
 * 
 * @return 0 if everything is all right, an error code otherwise
 */

int arts_init();

/**
 * disconnects from the sound server and frees the aRts C API internals
 */
void arts_free();

/**
 * converts an error code to a human readable error message
 *
 * @param errorcode the errorcode (from another arts function that failed)
 * @returns a text string with the error message
 */
const char *arts_error_text(int errorcode);

/**
 * open a stream for playing
 *
 * @param rate the sampling rate (something like 44100)
 * @param bits how many bits each sample has (8 or 16)
 * @param channels how many channels, 1 is mono, 2 is stereo
 * @param name the name of the stream (these will be used so that the user can
 *          assign streams to effects/mixer channels and similar)
 *
 * @return a stream
 */
arts_stream_t arts_play_stream(int rate, int bits, int channels, char *name);

/**
 * open a stream for recording
 *
 * @param rate the sampling rate (something like 44100)
 * @param bits how many bits each sample has (8 or 16)
 * @param channels how many channels, 1 is mono, 2 is stereo
 * @param name the name of the stream (these will be used so that the user can
 *          assign streams to effects/mixer channels and similar)
 *
 * @return a stream
 */
arts_stream_t arts_record_stream(int rate, int bits, int channels, char *name);

/**
 * close a stream
 */
void arts_close_stream(arts_stream_t stream);

/**
 * read samples from stream
 *
 * @param stream a previously opened record stream
 * @param buffer a buffer with sample data
 * @param count the number of bytes contained in the buffer
 *
 * @returns number of read bytes on success or error code
 */
int arts_read(arts_stream_t stream, void *buffer, int count);

/**
 * write samples to to stream
 *
 * @param stream a previously opened play stream
 * @param buffer a buffer with sample data
 * @param count the number of bytes contained in the buffer
 *
 * @returns number of written bytes on success or error code
 */
int arts_write(arts_stream_t stream, void *buffer, int count);

#endif /* ARTSC_H */
