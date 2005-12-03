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

#ifndef KPRINTACTION_H
#define KPRINTACTION_H

#include <kaction.h>

class KPrinter;

class KDEPRINT_EXPORT KPrintAction : public KActionMenu
{
	Q_OBJECT
public:
	enum PrinterType { All, Regular, Specials };

	KPrintAction(const QString& text, PrinterType type = All, QWidget *parentWidget = 0, KActionCollection *parent = 0, const char *name = 0);
	KPrintAction(const QString& text, const QIcon& icon, PrinterType type = All, QWidget *parentWidget = 0, KActionCollection *parent = 0, const char *name = 0);
	KPrintAction(const QString& text, const QString& icon, PrinterType type = All, QWidget *parentWidget = 0, KActionCollection *parent = 0, const char *name = 0);
	virtual ~KPrintAction();

	static KPrintAction* exportAll(QWidget *parentWidget = 0, KActionCollection *parent = 0, const char *name = 0);
	static KPrintAction* exportRegular(QWidget *parentWidget = 0, KActionCollection *parent = 0, const char *name = 0);
	static KPrintAction* exportSpecial(QWidget *parentWidget = 0, KActionCollection  *parent = 0, const char *name = 0);

signals:
	void print(KPrinter*);

protected slots:
	void slotAboutToShow();
	void slotActivated(int);

protected:
	void initialize(PrinterType type, QWidget *parentWidget);

private:
	class KPrintActionPrivate;
	KPrintActionPrivate	*d;
};

#endif
