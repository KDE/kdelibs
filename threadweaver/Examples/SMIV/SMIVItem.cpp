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

#include "SMIVItem.h"

#include <QtCore/QStringList>
#include <QtGui/QImage>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>

#include <DebuggingAids.h>
#include <ResourceRestrictionPolicy.h>

#include "SMIVItemDelegate.h"
#include "FileLoaderJob.h"
#include "QImageLoaderJob.h"

static QueuePolicy* resourceRestriction()
{
    static ResourceRestrictionPolicy policy( 4 );
    return &policy;
}


SMIVItem::SMIVItem ( Weaver *weaver,
                     const QString& path,  QObject *parent )
    : QObject ( parent )
    , m_path ( path )
    , m_weaver ( weaver )
{
    QFileInfo fi ( path );
    if ( fi.isFile() && fi.isReadable() )
    {
        m_sequence = new JobSequence ( this );

        m_name = fi.baseName();
        m_desc2 = fi.absoluteFilePath();
        m_fileloader = new FileLoaderJob ( fi.absoluteFilePath(),  this );
        m_fileloader->setObjectName ( tr ( "load file: " ) + fi.baseName() );
        connect ( m_fileloader,  SIGNAL ( done( ThreadWeaver::Job* ) ),
                  SLOT ( fileLoaderReady ( ThreadWeaver::Job* ) ) );
        m_fileloader->assignQueuePolicy( resourceRestriction() );
        m_imageloader = new QImageLoaderJob ( m_fileloader,  this );
        connect ( m_imageloader,  SIGNAL ( done( ThreadWeaver::Job* ) ),
                  SLOT ( imageLoaderReady ( ThreadWeaver::Job* ) ) );
        m_imageloader->setObjectName( tr( "load image: " ) + fi.baseName() );
        m_thumb = new ComputeThumbNailJob ( m_imageloader,  this );
        connect ( m_thumb,  SIGNAL ( done( ThreadWeaver::Job* ) ),
                  SLOT ( computeThumbReady ( ThreadWeaver::Job* ) ) );
        m_thumb->setObjectName ( tr( "scale image: " ) + fi.baseName() );
        m_sequence->addJob ( m_fileloader );
        m_sequence->addJob ( m_imageloader );
        m_sequence->addJob ( m_thumb );
        weaver->enqueue ( m_sequence );
    } else {
        // in this wee little program, we just ignore that we cannot access the file
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

void SMIVItem::fileLoaderReady( ThreadWeaver::Job* )
{
    debug ( 3, "SMIVItem::fileLoaderReady: %s loaded.\n",
            qPrintable ( m_name ) );
}

void SMIVItem::imageLoaderReady( ThreadWeaver::Job* )
{
    debug ( 3, "SMIVItem::imageLoaderReady: %s processed.\n",
            qPrintable ( m_name ) );
    // freem the memory held by the file loader, it is now redundant:
    m_fileloader->freeMemory();
    // this event has to be receive *before* computeThumbReady:
    P_ASSERT ( m_imageloader != 0 );
    QSize size = m_imageloader->image().size();
    m_desc1 = QString("%1x%2 Pixels")
              .arg( size.width() )
              .arg( size.height() );
}

void SMIVItem::computeThumbReady( ThreadWeaver::Job* )
{
    debug ( 3, "SMIVItem::computeThumbReady: %s scaled.\n",
            qPrintable ( m_name ) );
    //   delete m_imageloader; m_imageloader = 0;
    m_imageloader->resetImageData();
    emit ( thumbReady ( this ) );
}

QImage SMIVItem::thumb() const
{
    P_ASSERT ( m_thumb->isFinished() );
    return m_thumb->thumb();
}

#include "SMIVItem.moc"
