#include "qimageloaderjob.h"

namespace ThreadWeaver {

    QImageLoaderJob::QImageLoaderJob (FileLoaderJob* file, QObject* parent,
                                      const char* name)
        : Job (file, parent, name),
          m_file (file)
    {
    }

    QImageLoaderJob::~QImageLoaderJob()
    {
    }

    const QImage* QImageLoaderJob::image() const
    {
        if ( isFinished() && !m_image.isNull() )
        {
            return &m_image;
        } else {
            return 0;
        }
    }

    void QImageLoaderJob::run()
    {
        m_image.loadFromData ( (const uchar*) m_file->data(), m_file->size() );
    }

}
