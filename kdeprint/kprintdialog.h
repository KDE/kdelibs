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

class KPrintDialog : public KDialog, public KPReloadObject
{
	Q_OBJECT
public:
	KPrintDialog(QWidget *parent = 0, const char *name = 0);
	~KPrintDialog();

	void setFlags(int f);
	void setDialogPages(QPtrList<KPrintDialogPage> *pages);
	KPrinter* printer() const;

	static KPrintDialog* printerDialog(KPrinter*, QWidget*, const QString& caption = QString::null, bool forceExpand = false);

signals:
	void printRequested(KPrinter*);

protected slots:
	void slotPrinterSelected(int);
	void slotProperties();
	void slotSetDefault();
	void slotOptions();
	virtual void done(int);
	void slotWizard();
	void slotExtensionClicked();
	void slotToggleFilter(bool);
	void slotHelp();
	void slotOutputFileSelected(const QString&);
	void slotUpdatePossible( bool );

protected:
	bool checkOutputFile();
	void enableSpecial(bool on);
	void enableOutputFile(bool on);
	void setOutputFileExtension(const QString&);
	void reload();
	void configChanged();
	void expandDialog(bool on = true);
	void initialize( KPrinter* );
	void init();

protected:
	class KPrintDialogPrivate;
	KPrintDialogPrivate	*d;
};

#endif
