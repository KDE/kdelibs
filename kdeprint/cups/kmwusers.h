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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KMWUSERS_H
#define KMWUSERS_H

#include <kmwizardpage.h>

class KEditListBox;
class QComboBox;

class KMWUsers : public KMWizardPage
{
	Q_OBJECT
public:
	KMWUsers(QWidget *parent = 0, const char *name = 0);
	virtual ~KMWUsers();

	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

private:
	KEditListBox	*m_users;
	QComboBox	*m_type;
};

#endif
