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

#ifndef ARTS_AUDIOIO_H
#define ARTS_AUDIOIO_H

#include <startupmanager.h>
#include <string>

/*
 * The AudioIO class - it is used as follows:
 *
 *   1. check the settings with getParam, modify what you don't like
 *   2. call open -> print getInfo(lastError) if it failed (returned false)
 *   3. check whether the settings are still what you expected them to
 *      be (as the AudioIO class will maybe only know after open if the
 *      requested paramters can be achieved)
 */

namespace Arts {

class AudioIOPrivate;
class AudioIOFactory;

class AudioIO {
private:
	class AudioIOPrivate *d;

public:
	enum AudioParam {
/* the following has to be supported */
		samplingRate = 1,	/* usually 4000..48000 (Hz) */
		channels = 2,		/* usually 1 (mono) or 2 (stereo) */
		format = 3,			/*	8  = 8bit unsigned
							 *  16 = 16bit signed little endian
							 *	17 = 16bit signed big endian */

/* the following -can- be supported (value=-1 if they are not) */
		direction = 101,	/* 1 = read, 2 = write, 3 = read|write */
		fragmentCount = 102,/* usually 3..16 (to control latency) */
		fragmentSize = 103,	/* usually 256,512,...8192 (a 2^N value) */
		canRead = 104,		/* the amount of bytes that can be read */
		canWrite = 105,		/* the amount of bytes that can be written */
		selectFD = 106,		/* a filedescriptor to be used with select */
		autoDetect = 107,	/* 0 = don't use this as default 
							 * 1 or higher = preference of using this as default
							 *
							 * if nothing else is specified, aRts asks all
							 * available AudioIO classes for the autoDetect
							 * value and chooses the one which returned the
							 * highest number */

/* string parameters that have to be supported */
		lastError = 201,	/* the last error message as human readable text */

/* string parameters that -can- be supported */
		deviceName = 301,	/* name of the device to open */

/* class parameters: same semantics as above */
		name = 1201,		/* name of the driver (i.e. oss) */
		fullName = 1202		/* full name (i.e. Open Sound System) */
	};

	enum {
		directionRead = 1,
		directionWrite = 2,
		directionReadWrite = 3
	};

	AudioIO();
	virtual ~AudioIO();

	virtual void setParamStr(AudioParam param, const char *value);
	virtual void setParam(AudioParam param, int& value);
	virtual int getParam(AudioParam param);
	virtual const char *getParamStr(AudioParam param);

	virtual bool open() = 0;
	virtual void close() = 0;
	virtual int read(void *buffer, int size) = 0;
	virtual int write(void *buffer, int size) = 0;

/* ---- factory querying stuff ---- */
	static int queryAudioIOCount();
	static int queryAudioIOParam(int audioIO, AudioParam param);
	static const char *queryAudioIOParamStr(int audioIO, AudioParam param);

/* ---- factory stuff ---- */
	static AudioIO *createAudioIO(const char *name);
	static void addFactory(AudioIOFactory *factory);
	static void removeFactory(AudioIOFactory *factory);

protected:
	int& param(AudioParam param);
	std::string& paramStr(AudioParam param);
};

class AudioIOFactory : public StartupClass {
public:
	void startup();
	void shutdown();
	virtual AudioIO *createAudioIO() = 0;
	virtual const char *name() = 0;
	virtual const char *fullName() = 0;
};

};

#define REGISTER_AUDIO_IO(impl,implName,implFullName)				\
	static class AudioIOFactory ## impl : public AudioIOFactory {	\
	public:															\
		AudioIO *createAudioIO() { return new impl(); }				\
		virtual const char *name() { return implName; }				\
		virtual const char *fullName() { return implFullName; }		\
	} The_ ## impl ## _Factory /* <- add semicolon when calling this macro */

#endif /* ARTS_AUDIOIO_H */
