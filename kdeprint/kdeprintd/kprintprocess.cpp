/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kprintprocess.h"
#include <kapplication.h>
#include <klocale.h>
#include <QtCore/QFile>

KPrintProcess::KPrintProcess()
: KProcess()
{
	setOutputChannelMode( MergedChannels );
	connect(this, SIGNAL(readyReadStandardError()),
		SLOT(slotReadStandardError()));
	connect( this, SIGNAL( finished(int,QProcess::ExitStatus) ),
		SLOT( slotExited(int,QProcess::ExitStatus) ) );
	m_state = None;
}

KPrintProcess::~KPrintProcess()
{
	if ( !m_tempoutput.isEmpty() )
		QFile::remove( m_tempoutput );
	if ( m_tempfiles.size() > 0 )
		for ( QStringList::ConstIterator it=m_tempfiles.begin(); it!=m_tempfiles.end(); ++it )
			QFile::remove( *it );
}

QString KPrintProcess::errorMessage() const
{
	return m_buffer;
}

bool KPrintProcess::print()
{
	m_buffer.clear();
	m_state = Printing;
	start();
	return waitForStarted();
}

void KPrintProcess::slotReadStandardError()
{
	QByteArray	str = readAllStandardError().trimmed();
	m_buffer.append(QString::fromLocal8Bit(str.append("\n")));
}

void KPrintProcess::slotExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	switch ( m_state )
	{
		case Printing:
			if ( !m_output.isEmpty() )
			{
                                kDebug(500) << "Copying file" << m_output;
				QStringList args = QStringList() << "copy" << m_tempoutput << m_output;
				setProgram( "kfmclient", args );
				m_state = Finishing;
				m_buffer = i18n( "File transfer failed." );
				start();
				if ( waitForStarted() )
					return;
			}
		case Finishing:
			if ( exitStatus != NormalExit )
				emit printError( this, i18n( "Abnormal process termination (<b>%1</b>)." ,  m_command ) );
			else if ( exitCode != 0 )
				emit printError( this, i18n( "<b>%1</b>: execution failed with message:<p>%2</p>" ,  m_command ,  m_buffer ) );
			else
				emit printTerminated( this );
			break;
		default:
			emit printError( this, "Internal error, printing terminated in unexpected state. "
					"Report bug at <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a>." );
			break;
	}
}

#include "kprintprocess.moc"
