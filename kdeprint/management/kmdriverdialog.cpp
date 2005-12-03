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

#include "kmdriverdialog.h"
#include "driverview.h"

#include <klocale.h>
#include <kmessagebox.h>

KMDriverDialog::KMDriverDialog(QWidget *parent, const char *name)
: KDialogBase(KDialogBase::Swallow,i18n("Configure"),KDialogBase::Ok|KDialogBase::Cancel,KDialogBase::Ok,parent,name,true,false)
{
	m_view = new DriverView(0);
	setMainWidget(m_view);

	resize(400,450);
}

KMDriverDialog::~KMDriverDialog()
{
}

void KMDriverDialog::setDriver(DrMain *d)
{
	m_view->setDriver(d);
}

void KMDriverDialog::slotOk()
{
	if (m_view->hasConflict())
	{
		KMessageBox::error(this, "<qt>"+i18n("Some options are in conflict. You must resolve those conflicts "
		                                     "before continuing.")+"</qt>");
		return;
	}
	KDialogBase::slotOk();
}

#include "kmdriverdialog.moc"
