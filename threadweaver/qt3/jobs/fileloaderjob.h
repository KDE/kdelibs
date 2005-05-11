#ifndef FILELOADERJOB_H
#define FILELOADERJOB_H

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

#include <weaver.h>

namespace ThreadWeaver {

    /** This Job, when executed, loads a file on the file system into memory 
	(a raw byte buffer).  The raw data can be accessed using the data()
        method. 
	If the file has been successfully loaded, error returns zero. 
        Until the Job execution has finished, data() returns zero.
	data() is only valid after
        the done signal has been emitted, and it is valid until the object
        gets deleted. Objects of this class are not meant to be reused. 
	The allocated memory gets freed on destruction of the object. 
    */
    class FileLoaderJob : public Job
    {
        Q_OBJECT
    public:
        FileLoaderJob (QString filename, QObject* parent=0, const char* name=0);
        ~FileLoaderJob ();
        /** Return the raw data of the file.
            Unless the whole file is read or if an error happened, 
	    this returns zero. */
        const char* data () const;
	/** If an error occurred, return the error number. 
	    Otherwise, return zero. */
	const int error() const;
	/** Returns the file size in bytes. */
	const int size () const;
	/** Returns the filename. */
	const QString& filename() const;
    protected:
        void run();

	/** Filename. */
	const QString m_filename;
	/** Pointer to memory buffer. */
	char *m_data;
	/** stat information for the file. */
	struct stat m_statinfo;
	/** Keep the errno value. */
	int m_error;
    };
}

#endif
