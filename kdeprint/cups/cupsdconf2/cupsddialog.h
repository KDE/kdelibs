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

#ifndef	CUPSDDIALOG_H
#define	CUPSDDIALOG_H

#include <kdialogbase.h>
#include <qptrlist.h>

class CupsdPage;
struct CupsdConf;

class CupsdDialog : public KDialogBase
{
	Q_OBJECT
public:
	CupsdDialog(QWidget *parent = 0, const char *name = 0);
	~CupsdDialog();

	bool setConfigFile(const QString& filename);

	static void configure(const QString& filename = QString::null, QWidget *parent = 0);
        static bool restartServer(QString& msg);
	static int serverPid();
	static int serverOwner();

protected slots:
	void slotOk();
	void slotUser1();

protected:
	void addConfPage(CupsdPage*);
	void constructDialog();
	void restartServer();

private:
	QPtrList<CupsdPage>	pagelist_;
	CupsdConf		*conf_;
	QString			filename_;
};

#endif
