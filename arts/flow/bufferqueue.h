/*
 * BC - Status (2001-10-12): ByteBuffer, BufferQueue
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of libartsflow's
 * AudioSubSystem, and subject to change/disappearing due to optimization
 * work.
 */

#ifndef _BUFFERQUEUE_H
#define _BUFFERQUEUE_H

#include "thread.h"

#define _DEFAULT_CHUNK_SIZE 4096
#define _MAX_CHUNKS 3

namespace Arts
{

class ByteBuffer
{
	unsigned char* content;
	int _size;
	int _maxSize;
	int rp;

public:
	ByteBuffer() {
		_size = rp = 0;
                _maxSize = _DEFAULT_CHUNK_SIZE;
		content = new unsigned char[_DEFAULT_CHUNK_SIZE];
	}
	ByteBuffer(const void* s, int len) {
                _maxSize = _DEFAULT_CHUNK_SIZE;
        	content = new unsigned char[_DEFAULT_CHUNK_SIZE];
        	put(s, len);
	}

	~ByteBuffer() { delete content; }

	void put(const void* s, int len) {
		if ((_size = len) != 0)
			memcpy(content, s, len);
		rp = 0;
	}

	void* get()          { return content+rp; }
	void* reset()        { _size = 0; rp = 0; return content; }
	int push(int len)          { _size -= len; rp += len; return _size; }
	void set(int len)          { _size = len; rp = 0; }
	int size() const           { return _size; }
	int maxSize() const        { return _maxSize; }

        void setMaxSize(int size){
        	delete content;
		content = new unsigned char[size];
                _maxSize = size;
	}
};

///////////////////////////////////////////////////////////////////////////////

class BufferQueue
{
private:
	ByteBuffer bufs[_MAX_CHUNKS];
	int rp;
	int wp;
	Arts::Semaphore* sema_produced;
	Arts::Semaphore* sema_consumed;

	void semaReinit() {
		delete sema_consumed;
		delete sema_produced;
		sema_consumed = new Arts::Semaphore(0, _MAX_CHUNKS);
		sema_produced = new Arts::Semaphore(0, 0);
	}


public:
	BufferQueue() {
		rp = wp = 0;
		sema_consumed = new Arts::Semaphore(0, _MAX_CHUNKS);
		sema_produced = new Arts::Semaphore(0, 0);
	}

	~BufferQueue() {
		delete sema_consumed;
		delete sema_produced;
	}

	void write(void* data, int len);
	ByteBuffer* waitConsumed();
	void produced();

	ByteBuffer* waitProduced();
	void consumed();

	bool isEmpty() const       { return sema_produced->getValue() == 0; }
	int bufferedChunks() const { return sema_produced->getValue(); }
        int freeChunks() const     { return _MAX_CHUNKS - sema_produced->getValue(); }
	int maxChunks() const      { return _MAX_CHUNKS; }
	int chunkSize() const      { return bufs[0].maxSize(); }
	void clear()               { rp = wp = 0; semaReinit(); }
	void setChunkSize(int size){
        	for (int i=0; i < maxChunks(); i++)
			bufs[i].setMaxSize(size);
	}
};

///////////////////////////////////////////////////////////////////////////////

inline void BufferQueue::write(void* data, int len)
{
	sema_consumed->wait();
	bufs[wp].put(data, len);
	++wp %= _MAX_CHUNKS;
	sema_produced->post();
}

inline ByteBuffer* BufferQueue::waitConsumed()
{
	sema_consumed->wait();
	return &bufs[wp];
}

inline void BufferQueue::produced()
{
	++wp %= _MAX_CHUNKS;
	sema_produced->post();
}

inline ByteBuffer* BufferQueue::waitProduced()
{
	sema_produced->wait();
	return &bufs[rp];
}

inline void BufferQueue::consumed()
{
	++rp %=_MAX_CHUNKS;
	sema_consumed->post();
}

}

#endif
