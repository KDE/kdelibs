/* -*- C++ -*-

   This file implements the ComputeThumbNNailJob class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ComputeThumbNailJob.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <DebuggingAids.h>

#include "ComputeThumbNailJob.h"
#include "SMIVItemDelegate.h"

#define NO_QIMAGE_SCALED 0

ComputeThumbNailJob::ComputeThumbNailJob ( QImageLoaderJob *imageLoader,
                                           QObject *parent)
    : Job (parent),
      m_image (imageLoader)
{
    // addDependency ( imageLoader );
}

QImage ComputeThumbNailJob::thumb()
{
    P_ASSERT ( isFinished() &&  ! m_thumb.isNull() );
    return m_thumb;
}

void ComputeThumbNailJob::run()
{
    P_ASSERT ( m_image->isFinished() );

#if ! NO_QIMAGE_SCALED
    QImage im = m_image->image();
    if ( !im.isNull() )
    {
        m_thumb = im.scaled ( SMIVItemDelegate::ThumbWidth,
                              SMIVItemDelegate::ThumbHeight,
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation );
    } else {
        debug ( 0, "ComputeThumbNailJob::run: m_image returns a nil image.\n" );
    }
#else
    // dummy code to disable QImage::scaled (..):
    static QImage *im;
    if ( im == 0 )
    {
        GlobalMutex.lock();
        if ( im == 0 )
        {
            im = new QImage();
            im->load ( "thumbnail.jpg" );
        }
        GlobalMutex.unlock();
    }
    m_thumb = *im;
#endif
}
