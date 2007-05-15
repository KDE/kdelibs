/* -*- C++ -*-

   This file declares the SMIVItem class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVItem.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef SMIVItem_H
#define SMIVItem_H

#include <QtCore/QObject>

#include <QtGui/QImage>

#include <ThreadWeaver.h>
#include <JobSequence.h>
#include <FileLoaderJob.h>

#include "QImageLoaderJob.h"
#include "ComputeThumbNailJob.h"

using namespace ThreadWeaver;

class SMIVItem : public QObject
{
    Q_OBJECT
public:
    explicit SMIVItem( Weaver *weaver = 0,
              const QString& path  = QString(), QObject *parent = 0 );
    QString name() const;
    QString desc1() const;
    QString desc2() const;
    QImage thumb() const;
Q_SIGNALS:
    void thumbReady( SMIVItem* );
private Q_SLOTS:
    void fileLoaderReady ( ThreadWeaver::Job* );
    void imageLoaderReady ( ThreadWeaver::Job* );
    void computeThumbReady ( ThreadWeaver::Job* );
protected:
    QString m_path;
    QString m_name;
    QString m_desc1;
    QString m_desc2;
    Weaver *m_weaver;
    JobSequence *m_sequence;
    FileLoaderJob *m_fileloader;
    QImageLoaderJob *m_imageloader;
    ComputeThumbNailJob *m_thumb;
};

#endif // SMIVItem_H
