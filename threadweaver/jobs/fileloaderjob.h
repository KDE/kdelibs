#ifndef FILELOADERJOB_H
#define FILELOADERJOB_H

#include <weaver.h>

namespace ThreadWeaver {

    /** This Job, when executed, loads a file on the file system into memory 
	(a raw byte buffer).  The raw data can be accessed using the data()
        method. If the file cannot be loaded, data() returns zero. The same
        happens until the Job finished execution. data() is only valid after
        the done signal has been emitted.  data () is valid until the object
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
        const char* data ();
    protected:
        void run();

	/** Filename. */
	QString m_filename;
	/** Pointer to memory buffer. */
	char *m_data;
	/** False until file is loaded. */
	bool m_finished;
    };

}

#endif
