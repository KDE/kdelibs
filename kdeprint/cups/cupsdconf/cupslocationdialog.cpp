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

#include "cupslocationdialog.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <qwhatsthis.h>

#include "cupslocationgeneral.h"
#include "cupslocationaccess.h"
#include "cupsdconf.h"

CupsLocationDialog::CupsLocationDialog(CupsdConf *conf, QWidget *parent, const char *name)
	: QTabDialog(parent, name, true)
{
	conf_ = conf;

	general_ = new CupsLocationGeneral(conf_, this);
	addTab(general_, i18n("General"));
        general_->setInfos(conf);

	access_ = new CupsLocationAccess(this);
	addTab(access_, i18n("Access"));
        access_->setInfos(conf);

	setOkButton(i18n("OK"));
	setCancelButton(i18n("Cancel"));

	setHelpButton(i18n("Short Help"));
        connect(this, SIGNAL(helpButtonPressed()), SLOT(slotHelp()));

	setCaption(i18n("Add Resource"));
}

CupsLocationDialog::~CupsLocationDialog()
{
}

void CupsLocationDialog::loadLocation(CupsLocation *loc)
{
	general_->loadLocation(loc);
	access_->loadLocation(loc);

	setCaption(i18n("Resource \"%1\"").arg(loc->resourcename_));
}

void CupsLocationDialog::saveLocation(CupsLocation *loc)
{
	if (!general_->isValid())
		KMessageBox::error(this, i18n("You must specify a resource name!"));
	else
	{
		general_->saveLocation(loc);
		access_->saveLocation(loc);
	}
}

void CupsLocationDialog::done(int result)
{
	if (result == Accepted)
	{
		if (!general_->isValid())
		{
			KMessageBox::error(this, i18n("You must specify a resource name!"));
			return;
		}
	}
	QTabDialog::done(result);
}

void CupsLocationDialog::slotHelp()
{
	QWhatsThis::enterWhatsThisMode();
}
#include "cupslocationdialog.moc"
