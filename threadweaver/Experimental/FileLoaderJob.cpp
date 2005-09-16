/* -*- C++ -*-

   This file implements the FileLoaderJob class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: FileLoaderJob.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <errno.h>

#include "FileLoaderJob.h"

namespace ThreadWeaver {

    FileLoaderJob::FileLoaderJob (QString filename, QObject* parent)
        : FailableJob (parent),
          m_filename (filename),
          m_data (0),
          m_error (0)
    {
    }

    FileLoaderJob::~FileLoaderJob()
    {
        if (m_data != 0)
        {
            free (m_data);
        }
    }

    const int FileLoaderJob::error() const
    {
        return m_error;
    }

    bool FileLoaderJob::success () const
    {
        return ( error() == 0 );
    }

    const char* FileLoaderJob::data () const
    {   // we make sure data cannot be accesses until the file is completely
        // loaded, this way we do not need to mutex it:
        if ( isFinished() )
        {
            return m_data;
        } else {
            return 0;
        }
    }

    void FileLoaderJob::run()
    {
        int handle;

        // stat the file:
        if ( stat ( m_filename.toLocal8Bit(), &m_statinfo) == -1)
        {
            m_error = errno;
        } else {
            if (m_statinfo.st_size > 0)
            {   // stat successful:
                // create memory buffer:
                m_data = (char*) malloc (m_statinfo.st_size);
                if (m_data != 0)
                {   // malloc successful:
                    // open the file:
                    handle = open (m_filename.toLocal8Bit(), O_RDONLY);
                    if (handle == -1)
                    {
                        m_error = errno;
                    } else {
                        int bytesread = 0;
                        int chunksize = 1;
                        // read until EOF:
                        while (bytesread < m_statinfo.st_size && chunksize != 0)
                        {
                            chunksize = read (handle,
                                              m_data + bytesread,
                                              m_statinfo.st_size - bytesread);
                            if (chunksize == -1)
                            {
                                m_error = errno;
                                break;
                            } else {
                                if (chunksize > 0)
                                {
                                    bytesread += chunksize;
                                }
                            }
                        }
                        // close the file:
                        ::close (handle);
                    }
                }
            }
        }

        if (m_error != 0)
        {
            free (m_data);
            m_data = 0;
        }
    }

    const int FileLoaderJob::size () const
    {
        if (isFinished())
        {
            return m_statinfo.st_size;
        } else {
            return 0;
        }
    }

    const QString FileLoaderJob::filename() const
    {
        return m_filename;
    }
}

