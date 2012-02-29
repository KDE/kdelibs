/* -*- C++ -*-

   This file implements the FileLoaderJob class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: FileLoaderJob.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "FileLoaderJob.h"

#include <QtCore/QDebug>

namespace ThreadWeaver {

FileLoaderJob::FileLoaderJob (const QString &filename, QObject* parent)
    : Job (parent)
    , m_filename (filename)
    , m_data (0)
    , m_error (0)
{
}

FileLoaderJob::~FileLoaderJob()
{
    freeMemory();
}

const int FileLoaderJob::error() const
{
    return m_error;
}

bool FileLoaderJob::success () const
{
    return ( error() == 0 );
}

const char* FileLoaderJob::data () const
{   // we make sure data cannot be accesses until the file is completely
    // loaded, this way we do not need to mutex it:
    if ( isFinished() )
    {
	return m_data;
    } else {
	return 0;
    }
}

void FileLoaderJob::run()
{
    if( m_filename.isEmpty() ) {
        qDebug() << "FileLoaderJob::run: empty filename.";
        return;
    }
    m_file.setFileName( m_filename );
    if( m_file.open( QFile::ReadOnly ) ) {
        const int size = m_file.size();
        Q_ASSERT( m_data == 0 );
        if( m_data!=0 ) delete m_data;
        m_data = new char[size+1];

        if( m_file.read( m_data, size + 1) ) {
            m_error = 0;
            qDebug() << "FileLoaderJob::run: loaded.";
        } else {
            m_error = 1;
            qDebug() << "FileLoaderJob::run: failure loading.";
        }
    } else {
        m_error = 2;
        qDebug() << "FileLoaderJob::run: file does not exist or is not readable.";
    }
}

const int FileLoaderJob::size () const
{
    return m_file.size();
}

const QString FileLoaderJob::filename() const
{
    return m_filename;
}

void FileLoaderJob::freeMemory()
{
    delete m_data;
    m_data = 0;
}

}

#include "FileLoaderJob.moc"
