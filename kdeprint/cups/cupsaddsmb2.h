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

#ifndef CUPSADDSMB_H
#define CUPSADDSMB_H

#include <qobject.h>
#include <qstringlist.h>
#include <kprocess.h>
#include <kdialog.h>

class QProgressBar;
class SidePixmap;
class QPushButton;
class QLabel;
class KActiveLabel;
class QLineEdit;

class CupsAddSmb : public KDialog
{
	Q_OBJECT

public:
	enum State { None, Start, MkDir, Copy, AddDriver, AddPrinter };
	CupsAddSmb(QWidget *parent = 0, const char *name = 0);
	~CupsAddSmb();

	static bool exportDest(const QString& dest, const QString& datadir);

protected slots:
	void slotReceived(KProcess*, char*, int);
	void doNextAction();
	void slotProcessExited(KProcess*);
	void slotActionClicked();

protected:
	void checkActionStatus();
	void nextAction();
	bool startProcess();
	bool doExport();
	bool doInstall();
	void showError(const QString& msg);

private:
	KProcess	m_proc;
	QStringList	m_buffer;
	int			m_state;
	QStringList	m_actions;
	int			m_actionindex;
	bool		m_status;
	QProgressBar	*m_bar;
	QString		m_dest;
	SidePixmap	*m_side;
	QPushButton	*m_doit, *m_cancel;
	KActiveLabel	*m_text;
	QLabel *m_textinfo;
	QLineEdit *m_logined, *m_passwded, *m_servered;
	QString	m_datadir;
};

#endif
