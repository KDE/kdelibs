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

#ifndef ESCPWIDGET_H
#define ESCPWIDGET_H

#include <qwidget.h>
#include <kprocess.h>
#include <kurl.h>

class QLabel;
class QCheckBox;

class EscpWidget : public QWidget
{
	Q_OBJECT

public:
	EscpWidget(QWidget *parent = 0, const char *name = 0);
	void setDevice(const QString&);
	void setPrinterName(const QString&);

protected slots:
	void slotReceivedStdout(KProcess*, char*, int);
	void slotReceivedStderr(KProcess*, char*, int);
	void slotProcessExited(KProcess*);
	void slotButtonClicked();

protected:
	void startCommand(const QString& arg);

private:
	KProcess	m_proc;
	KURL		m_deviceURL;
	QString		m_errorbuffer, m_outbuffer;
	QLabel		*m_printer, *m_device;
	QCheckBox	*m_useraw;
	bool		m_hasoutput;
};

#endif
