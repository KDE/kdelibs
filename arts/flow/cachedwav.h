#ifndef CACHEDWAV_H
#include "config.h"
#ifdef HAVE_LIBAUDIOFILE
#include "cache.h"
#include <sys/stat.h>
#include <unistd.h>

class CachedWav : public CachedObject
{
protected:
	struct stat oldstat;
	std::string filename;
	bool initOk;

	CachedWav(Cache *cache, std::string filename);
	~CachedWav();

	typedef unsigned char uchar;
public:
	double samplingRate;
	long bufferSize;
	int channelCount;
	int sampleWidth;
	unsigned char *buffer;

	static CachedWav *load(Cache *cache, std::string filename);
	/**
	 * validity test for the cache - returns false if the object is having
	 * reflecting the correct contents anymore (e.g. if the file on the
	 * disk has changed), and there is no point in keeping it in the cache any
	 * longer
	 */
	bool isValid();
	/**
	 * memory usage for the cache
	 */
	int memoryUsage();
};

#endif
#endif
