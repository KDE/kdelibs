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

#ifndef EDITENTRYDIALOG_H
#define EDITENTRYDIALOG_H

#include <kdialogbase.h>
#include "printcapentry.h"

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QComboBox;
class Q3ListView;
class Q3ListViewItem;
class QListviewItem;
class QStackedWidget;

class EditEntryDialog : public KDialogBase
{
	Q_OBJECT
public:
	EditEntryDialog(PrintcapEntry *entry, QWidget *parent = 0, const char *name = 0);

	void fillEntry(PrintcapEntry *entry);

protected slots:
	void slotItemSelected(Q3ListViewItem*);
	void slotChanged();
	void slotTypeChanged(int);

protected:
	Field createField();

private:
	QMap<QString,Field>	m_fields;
	QLineEdit	*m_name, *m_string, *m_aliases;
	QCheckBox	*m_boolean;
	QComboBox	*m_type;
	QSpinBox	*m_number;
	Q3ListView	*m_view;
	QStackedWidget	*m_stack;
	QString		m_current;
	bool		m_block;
};

#endif
