/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *  Copyright (C) 2004  Dominique Devriese <devriese@kde.org>
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

#include "kxmlcommandselector.h"
#include "kmconfigdialog.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"
#include "kmmainview.h"

#include <kmessagebox.h>
#include <kdialogbase.h>
#include <klocale.h>

extern "C"
{
	int add_printer_wizard(QWidget *parent)
	{
                return kdeprint_management_add_printer_wizard( parent );
	}

	bool config_dialog(QWidget *parent)
	{
		KMConfigDialog	dlg(parent);
		return dlg.exec();
	}

	QString select_command( QWidget* parent )
	{
		KDialogBase dlg( parent, 0, true, i18n( "Select Command" ), KDialogBase::Ok|KDialogBase::Cancel );
		KXmlCommandSelector *xmlSel = new KXmlCommandSelector( false, &dlg, "CommandSelector", &dlg );
		dlg.setMainWidget( xmlSel );
		if ( dlg.exec() )
			return xmlSel->command();
		return QString::null;
	}
}

