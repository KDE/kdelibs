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

#ifndef KMINFOPAGE_H
#define KMINFOPAGE_H

#include "kmprinterpage.h"
#include <qwidget.h>

class QLabel;

class KMInfoPage : public QWidget, public KMPrinterPage
{
public:
	KMInfoPage(QWidget *parent = 0, const char *name = 0);
	~KMInfoPage();

	void setPrinter(KMPrinter *p);

protected:
	QLabel	*m_title, *m_titlepixmap;
	QLabel	*m_location, *m_description, *m_uri, *m_model, *m_type, *m_state, *m_device;
	QLabel	*m_devlabel;
};

#endif
