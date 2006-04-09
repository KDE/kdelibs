/* -*- C++ -*-

   This file declares the QImageLoaderJob class. It loads the content of the
   raw data into a QImage.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: QImageLoaderJob.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef QIMAGELOADERJOB_H
#define QIMAGELOADERJOB_H

#include <QImage>

#include <Job.h>

#include "FileLoaderJob.h"

namespace ThreadWeaver {

    /** This class loads raw data into a QImage.
        The data will be taken from a FileLoaderJob object.
    */
    class QImageLoaderJob : public Job
    {
    public:
        /** Create a job to load an image from the data loaded into file.
            If file has not been finished at this point, it is considered a
            dependency.
        */
        QImageLoaderJob (FileLoaderJob* file, QObject* parent=0);
        ~QImageLoaderJob ();
        /** Return the image. If it is not completely created, null will be
            returned. If an error occured (for example, the data in file is a
            corrupted image), null will be returned, too. */
        QImage image() const;
    protected:
        void run();

        QImage m_image;
        FileLoaderJob *m_file;
    };

}

#endif // QIMAGELOADERJOB_H

