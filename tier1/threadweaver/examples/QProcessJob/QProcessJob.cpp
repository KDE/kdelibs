/* -*- C++ -*-

This file implements a class that executes QProcesses as a Job.

$ Author: Mirko Boehm $
$ Copyright: (C) 2006, 2007 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#include "QProcessJob.h"

using namespace ThreadWeaver;

QProcessJob::QProcessJob ( QObject * parent )
    : Job ( parent )
{
}

void QProcessJob::run()
{
    QProcess m_process;

    QStringList args;
    args.append ( "+%H:%m:%S %N" );
    m_process.start( "date", args );

    if (  ! m_process.waitForFinished ( -1 ) )
    {
        return;
    }

    m_result = m_process.readAll();

    // qDebug ( "'date' returned %s", result.constData() );
}

QString QProcessJob::result()
{
    Q_ASSERT ( isFinished () );
    return m_result;
}

#include "QProcessJob.moc"
