/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KPRINTPROCESS_H
#define KPRINTPROCESS_H

#include <kprocess.h>

class KPrintProcess : public KShellProcess
{
	Q_OBJECT
public:
	KPrintProcess();
	~KPrintProcess();

	bool print();
	QString errorMessage() const;

signals:
	void passwordRequested(KProcess*, const QString&);

protected slots:
	void slotReceivedStderr(KProcess*, char*, int);

private:
	QString	m_buffer;
};

#endif
