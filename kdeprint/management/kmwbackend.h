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

#ifndef KMWBACKEND_H
#define KMWBACKEND_H

#include "kmwizardpage.h"
#include <qmap.h>

class QButtonGroup;
class QVBoxLayout;

class KMWBackend : public KMWizardPage
{
public:
	KMWBackend(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

	void addBackend(int ID = -1, const QString& txt = QString::null, bool on = true, int nextpage = -1);
	void enableBackend(int ID, bool on = true);

private:
	QButtonGroup	*m_buttons;
	QVBoxLayout	*m_layout;
	// keep a map between button ID and the real next page to switch to. This enables
	// to have different backends switching to the same page (like backends requiring
	// a password). If the next page is not given when adding the backend, the ID is
	// used by default.
	QMap<int,int>	m_map;
	int 		m_count;
};

#endif
