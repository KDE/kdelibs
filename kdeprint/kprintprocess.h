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

#ifndef KPRINTPROCESS_H
#define KPRINTPROCESS_H

#include <kprocess.h>
#include <qstringlist.h>

class KPrintProcess : public KShellProcess
{
	Q_OBJECT
public:
	KPrintProcess();
	~KPrintProcess();

	bool print();
	QString errorMessage() const;

	void setOutput( const QString& output );
	const QString& output() const;
	void setTempOutput( const QString& output );
	const QString& tempOutput() const;
	void setTempFiles( const QStringList& files );
	const QStringList& tempFiles() const;
	void setCommand( const QString& cmd );
	const QString& command() const;

	enum State { None = 0, Printing, Finishing };
	int state() const;

signals:
	void printTerminated( KPrintProcess* );
	void printError( KPrintProcess*, const QString& );

protected slots:
	void slotReceivedStderr(KProcess*, char*, int);
	void slotExited( KProcess* );

private:
	QString	m_buffer;
	QStringList m_tempfiles;
	QString m_output, m_tempoutput, m_command;
	int m_state;
};

inline const QString& KPrintProcess::output() const
{ return m_output; }

inline const QString& KPrintProcess::tempOutput() const
{ return m_tempoutput; }

inline const QStringList& KPrintProcess::tempFiles() const
{ return m_tempfiles; }

inline const QString& KPrintProcess::command() const
{ return m_command; }

inline void KPrintProcess::setOutput( const QString& s )
{ m_output = s; }

inline void KPrintProcess::setTempOutput( const QString& s )
{ m_tempoutput = s; }

inline void KPrintProcess::setTempFiles( const QStringList& l )
{ m_tempfiles = l; }

inline void KPrintProcess::setCommand( const QString& c )
{ m_command = c; }

inline int KPrintProcess::state() const
{ return m_state; }

#endif
