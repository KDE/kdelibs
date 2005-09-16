/* -*- C++ -*-

   This file declares the FileLoaderJob class. FileLoaderJob loads the content
   of a file on the local filesystem into memory.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: FileLoaderJob.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef FILELOADERJOB_H
#define FILELOADERJOB_H

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

#include <WeaverImpl.h>
#include "FailableJob.h"

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
    class FileLoaderJob : public FailableJob
    {
        Q_OBJECT
    public:
        explicit FileLoaderJob (QString filename, QObject* parent=0);
        ~FileLoaderJob ();
        /** Return true if the operation succeeded. */
        bool success () const;
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
	const QString filename() const;
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
