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

#ifndef KMPROPERTYPAGE_H
#define KMPROPERTYPAGE_H

#include "cjanuswidget.h"
#include "kmprinterpage.h"
#include <kdeprint/kpreloadobject.h>

#include <qptrlist.h>

class KMPropWidget;
class KMPrinter;

class KMPropertyPage : public CJanusWidget, public KMPrinterPage, public KPReloadObject
{
	Q_OBJECT
public:
	KMPropertyPage(QWidget *parent = 0, const char *name = 0);
	~KMPropertyPage();

	void addPropPage(KMPropWidget*);
	void setPrinter(KMPrinter*);

protected slots:
	void slotEnable(bool);
	void initialize();

protected:
	void reload();

private:
	QPtrList<KMPropWidget>	m_widgets;
};

#endif
