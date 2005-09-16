/* -*- C++ -*-

   This file implements the QImageLoader class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: QImageLoaderJob.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <DebuggingAids.h>

#include "QImageLoaderJob.h"

namespace ThreadWeaver {

    QImageLoaderJob::QImageLoaderJob (FileLoaderJob* file, QObject* parent)
        : Job (parent),
          m_file (file)
    {
        // addDependency ( file );
    }

    QImageLoaderJob::~QImageLoaderJob()
    {
    }

    QImage QImageLoaderJob::image() const
    {
        P_ASSERT ( isFinished() );
        return m_image;
    }

    void QImageLoaderJob::run()
    {
        P_ASSERT( m_file->isFinished() );

        if (m_file->data() == 0)
	{
	    debug (0, "QImageLoaderJob::run: data array is empty.\n" );
	} else {
	    m_image.loadFromData ( (const uchar*) m_file->data(), m_file->size() );
	}
    }

}
