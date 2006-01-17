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

#ifndef KMCONFIGDIALOG_H
#define KMCONFIGDIALOG_H

#include <kdialogbase.h>
#include <q3ptrlist.h>

#include "kmconfigpage.h"

class KDEPRINT_EXPORT KMConfigDialog : public KDialogBase
{
	Q_OBJECT
public:
	KMConfigDialog(QWidget *parent = 0, const char *name = 0);

	void addConfigPage(KMConfigPage*);

protected Q_SLOTS:
	void slotOk();

private:
	QList<KMConfigPage*>	m_pages;
};

#endif
