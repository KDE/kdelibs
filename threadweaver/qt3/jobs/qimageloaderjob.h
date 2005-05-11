#ifndef QIMAGELOADERJOB_H
#define QIMAGELOADERJOB_H

#include <qimage.h>

#include <weaver.h>

#include "fileloaderjob.h"

namespace ThreadWeaver {

    /** This class loads raw data into a QImage.
        The data will be taken from a FileLoaderJob object.
    */
    class QImageLoaderJob : public Job
    {
    public:
        /** Create a job to load an image from the data loaded into file.
            If file has not been finished at this point, it is considered a
            dependancy.
        */
        QImageLoaderJob (FileLoaderJob* file, QObject* parent=0,
                         const char* name=0);
        ~QImageLoaderJob ();
        /** Return the image. If it is not completely created, null will be
            returned. If an error occured (for example, the data in file is a
            corrupted image), null will be returned, too. */
        const QImage* image() const;
    protected:
        void run();

        QImage m_image;
        FileLoaderJob *m_file;
    };

}
#endif
