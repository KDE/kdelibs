/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef KPRINTDIALOG_H
#define KPRINTDIALOG_H

#include <kdialog.h>
#include <qptrlist.h>
#include <kdeprint/kpreloadobject.h>

class KPrinter;
class KPrintDialogPage;
class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLineEdit;
class QVBox;
class PluginComboBox;
class KFileList;
class KURLRequester;

class KPrintDialog : public KDialog, public KPReloadObject
{
	Q_OBJECT
public:
	KPrintDialog(QWidget *parent = 0, const char *name = 0);
	~KPrintDialog();

	void setFlags(int f);
	void setDialogPages(QPtrList<KPrintDialogPage> *pages);
	void initialize(KPrinter*);
	KPrinter* printer() const;

	static bool printerSetup(KPrinter*, QWidget*, const QString& caption = QString::null);

protected slots:
	void slotPrinterSelected(int);
	void slotProperties();
	void slotSetDefault();
	void slotOptions();
	virtual void done(int);
	void slotWizard();

protected:
	bool checkOutputFile();
	void enableSpecial(bool on);
	void enableOutputFile(bool on);
	void setOutputFileExtension(const QString&);
	void reload();

protected:
	QLabel	*m_type, *m_state, *m_comment, *m_location, *m_cmdlabel, *m_filelabel;
	QPushButton	*m_properties, *m_default, *m_options, *m_ok, *m_wizard;
	QCheckBox	*m_preview;
	QLineEdit	*m_cmd;
	QComboBox	*m_printers;
	QVBox		*m_dummy;
	PluginComboBox	*m_plugin;
	KFileList	*m_fileselect;
	KURLRequester	*m_file;

	QPtrList<KPrintDialogPage>	m_pages;
	KPrinter		*m_printer;
};

inline KPrinter* KPrintDialog::printer() const
{ return m_printer; }

#endif
