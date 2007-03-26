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
: K3ShellProcess()
{
	// redirect everything to a single buffer
	connect(this,SIGNAL(receivedStdout(K3Process*,char*,int)),SLOT(slotReceivedStderr(K3Process*,char*,int)));
	connect(this,SIGNAL(receivedStderr(K3Process*,char*,int)),SLOT(slotReceivedStderr(K3Process*,char*,int)));
	connect( this, SIGNAL( processExited( K3Process* ) ), SLOT( slotExited( K3Process* ) ) );
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
	return start(NotifyOnExit,All);
}

void KPrintProcess::slotReceivedStderr(K3Process *proc, char *buf, int len)
{
	if (proc == this)
	{
		QByteArray	str = QByteArray(buf,len).trimmed();
		m_buffer.append(str.append("\n"));
	}
}

void KPrintProcess::slotExited( K3Process* )
{
	switch ( m_state )
	{
		case Printing:
			if ( !m_output.isEmpty() )
			{
				clearArguments();
				*this << "kfmclient" << "copy" << m_tempoutput << m_output;
				m_state = Finishing;
				m_buffer = i18n( "File transfer failed." );
				if ( start( NotifyOnExit ) )
					return;
			}
		case Finishing:
			if ( !normalExit() )
				emit printError( this, i18n( "Abnormal process termination (<b>%1</b>)." ,  m_command ) );
			else if ( exitStatus() != 0 )
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
