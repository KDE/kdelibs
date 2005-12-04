/* -*- C++ -*-

   This file implements the SMIVItem class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVItem.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QStringList>
#include <QImage>
#include <QTimer>
#include <QFileInfo>

#include <DebuggingAids.h>

#include "SMIVItemDelegate.h"
#include "FileLoaderJob.h"
#include "QImageLoaderJob.h"

#include "SMIVItem.h"

SMIVItem::SMIVItem ( Weaver *weaver,
                     const QString& path,  QObject *parent )
    : QObject ( parent ),
      m_path ( path ),
      m_weaver ( weaver )
{
    QFileInfo fi ( path );
    if ( fi.isFile() && fi.isReadable() )
    {
        m_sequence = new JobSequence ( this );

        m_name = fi.baseName();
        m_desc2 = fi.absoluteFilePath();
        m_fileloader = new FileLoaderJob ( fi.absoluteFilePath(),  this );
        connect ( m_fileloader,  SIGNAL ( done( Job* ) ),
                  SLOT ( fileLoaderReady ( Job* ) ) );
        m_imageloader = new QImageLoaderJob ( m_fileloader,  this );
        connect ( m_imageloader,  SIGNAL ( done( Job* ) ),
                  SLOT ( imageLoaderReady ( Job* ) ) );
        m_thumb = new ComputeThumbNailJob ( m_imageloader,  this );
        connect ( m_thumb,  SIGNAL ( done( Job* ) ),
                  SLOT ( computeThumbReady ( Job* ) ) );

        m_sequence->append ( m_fileloader );
        connect ( m_fileloader,  SIGNAL ( failed ( Job* ) ),
                  m_sequence,  SLOT ( stop ( Job* ) ) );
        m_sequence->append ( m_imageloader );
        m_sequence->append ( m_thumb );
        weaver->enqueue ( m_sequence );
//         weaver->enqueue( m_fileloader );
//         weaver->enqueue ( m_imageloader );
//         weaver->enqueue ( m_thumb );
    } else {
        // @TODO: error handling
    }
}

QString SMIVItem::name() const
{
    return m_name;
}

QString SMIVItem::desc1() const
{
    return m_desc1;
}

QString SMIVItem::desc2() const
{
    return m_desc2;
}

void SMIVItem::fileLoaderReady( Job* )
{
    debug ( 3, "SMIVItem::fileLoaderReady: %s loaded.\n",
            qPrintable ( m_name ) );
}

void SMIVItem::imageLoaderReady( Job* )
{
    debug ( 3, "SMIVItem::imageLoaderReady: %s processed.\n",
            qPrintable ( m_name ) );
    delete m_fileloader; m_fileloader = 0;
    // this event has to be receive *before* computeThumbReady:
    P_ASSERT ( m_imageloader != 0 );
    QSize size = m_imageloader->image().size();
    m_desc1 = QString("%1x%2 Pixels")
              .arg( size.width() )
              .arg( size.height() );
}

void SMIVItem::computeThumbReady( Job* )
{
    debug ( 3, "SMIVItem::computeThumbReady: %s scaled.\n",
            qPrintable ( m_name ) );
    delete m_imageloader; m_imageloader = 0;
    emit ( thumbReady ( this ) );
}

QImage SMIVItem::thumb() const
{
    P_ASSERT ( m_thumb->isFinished() );
    return m_thumb->thumb();
}
