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

#ifdef __cplusplus
extern "C" {
#endif

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
#define ARTS_E_NOSTREAM     ( -3 )
#define ARTS_E_NOINIT       ( -4 )
#define ARTS_E_NOIMPL       ( -5 )

/**
 * the values for stream parameters
 *
 * @see arts_parameter_t
 */
enum arts_parameter_t_enum {
    ARTS_P_BUFFER_SIZE = 1,
    ARTS_P_BUFFER_TIME = 2,
    ARTS_P_BUFFER_SPACE = 3,
    ARTS_P_SERVER_LATENCY = 4,
    ARTS_P_TOTAL_LATENCY = 5,
    ARTS_P_BLOCKING = 6,
    ARTS_P_PACKET_SIZE = 7,
    ARTS_P_PACKET_COUNT = 8,
	ARTS_P_PACKET_SETTINGS = 9
};

/**
 * parameters for streams
 *
 * @li ARTS_P_BUFFER_SIZE (rw)
 *   The size of the internal buffers used for streaming to the server - this
 *   directly affects the latency that will occur. If you never set it
 *   explicitely, this value defaults to at least 65536 (64kb). Generally,
 *   it is important to know that the server itself gives some constraints
 *   which makes buffer sizes below a certain value impossible. So setting
 *   this parameter will always result in either getting what you wanted,
 *   or a larger streaming buffer due to server limitations.
 *
 * @li ARTS_P_BUFFER_TIME (rw)
 *   The time the buffer used for streaming to the server takes to play in
 *   milliseconds. This is just a more human readable method to set the buffer
 *   size, so setting ARTS_P_BUFFER_SIZE affects this parameter and the other
 *   way round. As aRts chooses reasonable buffer sizes for streaming (rather
 *   3*16kb than 40234 bytes), setting this parameter will often end up with
 *   a slightly larger value than you requested.
 *
 * @li ARTS_P_BUFFER_SPACE (r)
 *   The amount of bytes that can be read/written without blocking (depending
 *   whether this is a record or play stream). As requesting this parameter
 *   does a few system calls (but no remote invocation) to verify that it is
 *   up-to-date, don't overuse it.
 *
 * @li ARTS_P_SERVER_LATENCY (r)
 *   The amount of latency the server creates (due to hardware buffering)
 *   in milliseconds.
 *
 * @li ARTS_P_TOTAL_LATENCY (r)
 *   The overall latency in milliseconds it takes (at most), from the time
 *   when you write a byte into a stream, until it gets played on the
 *   soundcard. This is simply a shortcut to the sum of ARTS_P_BUFFER_TIME
 *   and ARTS_P_SERVER_LATENCY.
 *
 * @li ARTS_P_BLOCKING (rw)
 *   If this parameter is 1 (the default), arts_read/arts_write will block
 *   when not all data can be read/written successfully, and wait until it
 *   works. If this parameter is 0, arts_read/arts_write will return
 *   the number of successfully read/written bytes immediately.
 *
 * @li ARTS_P_PACKET_SIZE (r)
 *   This returns the size of the packets used for buffering. The optimal
 *   size for arts_stream_write is always writing one packet. The buffering of
 *   streams works with audio packets. So the ARTS_P_BUFFER_SIZE parameter of
 *   streams (which specifies how many bytes of a stream are prebuffered),
 *   really consists of (ARTS_P_PACKET_SIZE) * (ARTS_P_PACKET_COUNT).
 *
 * @li ARTS_P_PACKET_COUNT (r)
 *   This returns the number of the packets are used for buffering. See
 *   ARTS_P_PACKET_SIZE for more.
 *
 * @li ARTS_P_PACKET_SETTINGS (rw)
 *   This is a way to configure packet size & packet count at the same time.
 *   The format is 0xCCCCSSSS, where 2^SSSS is the packet size, and CCCC is
 *   the packet count. Note that when writing this, you don't necessarily
 *   get the settings you requested.
 */
typedef enum arts_parameter_t_enum arts_parameter_t;

/**
 * initializes the aRts C API, and connects to the sound server
 *
 * @return 0 if everything is all right, an error code otherwise
 */

int arts_init(void);

/**
 * disconnects from the sound server and frees the aRts C API internals
 */
void arts_free(void);

/**
 * asks aRtsd to free the DSP device and return 1 if it was successful,
 * 0 if there were active non-suspendable modules
 */
int arts_suspend(void);

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
arts_stream_t arts_play_stream(int rate, int bits, int channels, const char *name);

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
arts_stream_t arts_record_stream(int rate, int bits, int channels, const char *name);

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
int arts_write(arts_stream_t stream, const void *buffer, int count);

/**
 * configure a parameter of a stream
 *
 * @param stream an opened record or play stream
 * @param parameter the parameter you want to modify
 * @param value the new value
 *
 * @returns the new value of the parameter (which may or may not be the value
 *          you wanted to have), or an error code if something went wrong
 */
int arts_stream_set(arts_stream_t stream, arts_parameter_t param, int value);

/**
 * query a parameter of a stream
 *
 * @param stream an opened record or play stream
 * @param parameter the parameter you want to query
 *
 * @returns the value of the parameter, or an error code
 */
int arts_stream_get(arts_stream_t stream, arts_parameter_t param);

#ifdef __cplusplus
}
#endif

#endif /* ARTSC_H */
