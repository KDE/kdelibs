/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#include "kprintdialogpage.h"

KPrintDialogPage::KPrintDialogPage(QWidget *parent, const char *name)
: QWidget(parent,name), m_printer(0), m_driver(0), m_ID(0), m_onlyreal(false)
{
}

KPrintDialogPage::KPrintDialogPage(KMPrinter *pr, DrMain *dr, QWidget *parent, const char *name)
: QWidget(parent,name), m_printer(pr), m_driver(dr), m_ID(0), m_onlyreal(false)
{
}

KPrintDialogPage::~KPrintDialogPage()
{
}

void KPrintDialogPage::setOptions(const QMap<QString,QString>&)
{
}

void KPrintDialogPage::getOptions(QMap<QString,QString>&, bool)
{
}

bool KPrintDialogPage::isValid(QString&)
{
	return true;
}
#include "kprintdialogpage.moc"
