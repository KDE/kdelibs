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

#ifndef AUDIOSUBSYS_H
#define AUDIOSUBSYS_H
#include <string>
#include "pipebuffer.h"
#include "startupmanager.h"

namespace Arts {

class ASProducer {
public:
	virtual void needMore() = 0;
};

// FIXME: haveMore won't be called currently
class ASConsumer {
public:
	virtual void haveMore() = 0;
};

class AudioSubSystemStart :public StartupClass {
protected:
	class AudioSubSystem *_instance;
public:
	inline AudioSubSystem *the() { return _instance; };
	void startup();
	void shutdown();
};

class AudioSubSystem {
	// unused, added just in case
	class AudioSubSystemPrivate *d;

	std::string _error;
	int audio_fd;
	char *fragment_buffer;
	int usageCount;

	int _fragmentCount;
	int _fragmentSize;
	int _samplingRate;
	int _channels;

	bool _running;
	bool _fullDuplex;
	PipeBuffer wBuffer, rBuffer;
	ASConsumer *consumer;
	ASProducer *producer;

	friend class AudioSubSystemStart;
	AudioSubSystem();

	void close();

public:
	enum { ioRead=1, ioWrite=2, ioExcept=4 };

	// singleton
	static AudioSubSystem *the();

	/*
	 * Currently, if you want to use the AudioSubSystem, you need to
	 * 
	 * 1. - attach one producer
	 *    - attach one consumer (only for full duplex)
	 *    - open the audio subsystem using open (watch the fd)
	 *    (in any order)
	 *
	 * 2. react on the callbacks you get for the producer
	 *
	 * 3. if you don't need the audio subsystem any longer, call detach
	 *    both, the producer and the cosumer.
	 *
	 * Be careful that you don't read/write from/to the audio subsystem
	 * when running() is not true.
	 */

	bool attachProducer(ASProducer *producer);
	bool attachConsumer(ASConsumer *consumer);

	void detachProducer();
	void detachConsumer();

	void fragmentSize(int size);
	int fragmentSize();

	void fragmentCount(int fragments);
	int fragmentCount();

	void samplingRate(int samplingrate);
	int samplingRate();

	void channels(int channels);
	int channels();

	void fullDuplex(bool newFullDuplex);
	bool fullDuplex();

	bool check();

	int open();
	const char *error();

	/*
	 * returncode of open: -1 indicates an error; otherwise, the returned
	 * filedescriptor must be watched using select calls, and, whenever
	 * fd is ready for something, handleIO must be called. 
	 *
	 * The type for handleIO must be set to ioRead if fd is ready for
	 * reading, ioWrite if fd is ready for writing, ioExcept if something
	 * special happend or any combination of these using bitwise or.
	 */
	void handleIO(int type);

	void read(void *buffer, int size);
	void write(void *buffer, int size);

	/*
	 * returns true as long as the audio subsystem is opened and active (that
	 * is, between successful opening, with attaching producer, and the first
	 * detachConsumer/detachProducer
	 */
	bool running();
};

};

#endif /* AUDIOSUBSYS_H */
