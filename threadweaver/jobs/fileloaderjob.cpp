#include <errno.h>

#include "fileloaderjob.h"

namespace ThreadWeaver {

    FileLoaderJob::FileLoaderJob (QString filename,
                                  QObject* parent,
                                  const char* name)
        : Job (parent, name),
          m_filename (filename),
          m_data (0),
          m_finished (false),
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

    const char* FileLoaderJob::data () const
    {   // we make sure data cannot be accesses until the file is completely
        // loaded, this way we do not need to mutex it:
        if (m_finished)
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
        if ( stat ( m_filename.local8Bit(), &m_statinfo) == -1)
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
                    handle = open (m_filename.local8Bit(), O_RDONLY);
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
        // set m_finished and return:
        m_finished = true;
    }

    const int FileLoaderJob::size () const
    {
        if (m_finished)
        {
            return m_statinfo.st_size;
        } else {
            return 0;
        }
    }

    const QString& FileLoaderJob::filename() const
    {
        return m_filename;
    }
}

#include "fileloaderjob.moc"
