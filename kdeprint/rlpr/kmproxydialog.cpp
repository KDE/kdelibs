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

#include "kmproxydialog.h"
#include "kmproxywidget.h"
#include "kmfactory.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>

KMProxyDialog::KMProxyDialog(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok, true)
{
	m_widget = new KMProxyWidget(this);
	setMainWidget(m_widget);

	resize(300,100);
	setCaption(i18n("RLPR Settings"));
}

bool KMProxyDialog::configure(QWidget *parent)
{
	KMProxyDialog	dlg(parent);
	dlg.m_widget->loadConfig(KMFactory::self()->printConfig());
	if (dlg.exec())
	{
		KConfig	*conf = KMFactory::self()->printConfig();
		dlg.m_widget->saveConfig(conf);
		conf->sync();
		return true;
	}
	return false;
}
