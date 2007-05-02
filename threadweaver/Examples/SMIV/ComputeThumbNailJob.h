/* -*- C++ -*-

   This file declares the ComputeThumbNailJob class. It scales images.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ComputeThumbNailJob.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef COMPUTETHUMBNAILJOB_H
#define COMPUTETHUMBNAILJOB_H

#include <QtGui/QImage>

#include "Job.h"

#include "QImageLoaderJob.h"

using namespace ThreadWeaver;

class ComputeThumbNailJob : public Job
{
    Q_OBJECT
public:
    explicit ComputeThumbNailJob ( QImageLoaderJob *imageLoader,
			  QObject *parent = 0);
    /** Returns the "ready made" thumbnail. */
  QImage thumb();

  /** reimpl */
  int priority () const;
protected:
    void run();
    QImage m_thumb;
    const QImageLoaderJob *m_image;
};

#endif // COMPUTETHUMBNAILJOB_H
