extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

#include "fileloaderjob.h"

namespace ThreadWeaver {

    FileLoaderJob::FileLoaderJob (QString filename,
                                  QObject* parent,
                                  const char* name)
        : Job (parent, name),
          m_filename (filename),
          m_data (0),
          m_finished (false)
    {

    }

    FileLoaderJob::~FileLoaderJob()
    {
        if (m_data != 0)
        {
            free (m_data);
        }
    }

    const char* FileLoaderJob::data ()
    {
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
        struct stat info;
        // stat the file:
        if ( stat ( m_filename.local8Bit(), &info) == 0
            && info.st_size > 0)
        {   // stat successful:
            // create memory buffer:
            m_data = (char*) malloc (info.st_size);
            if (m_data != 0)
            {   // malloc successful:
                // open the file:
                handle = open (m_filename.local8Bit(), O_RDONLY);
                if (handle != 0)
                {
                    int bytesread = 0;
                    int chunksize;
                    bool eof = false;
                    // read until EOF:
                    while (bytesread < info.st_size && eof == false)
                    {
                        chunksize = read (handle,
                                          m_data + bytesread,
                                          info.st_size - bytesread);
                        if (chunksize == 0)
                        {
                            eof = true;
                        } else {
                            bytesread += chunksize;
                        }
                    }
                    // close the file:
                    ::close (handle);
                } else {
                    free (m_data);
                    m_data = 0;
                }

            }
        }
        // set m_finished and return:
        m_finished = true;
    }
}

#include "fileloaderjob.moc"
