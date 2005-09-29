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

#ifndef KMSPECIALPRINTERDLG_H
#define KMSPECIALPRINTERDLG_H

#include <kdialogbase.h>

class KMPrinter;
class QLineEdit;
class QCheckBox;
class QComboBox;
class KIconButton;
class KXmlCommandSelector;

class KMSpecialPrinterDlg : public KDialogBase
{
	Q_OBJECT
public:
	KMSpecialPrinterDlg(QWidget *parent = 0, const char *name = 0);

	void setPrinter(KMPrinter*);
	KMPrinter* printer();

protected:
	bool checkSettings();

protected slots:
	void slotOk();
	void slotTextChanged(const QString &);

private:
	QLineEdit	*m_name, *m_description, *m_location, *m_extension;
	QComboBox	*m_mimetype;
	QCheckBox	*m_usefile;
	QStringList	m_mimelist;
	KIconButton	*m_icon;
	KXmlCommandSelector	*m_command;
};

#endif
