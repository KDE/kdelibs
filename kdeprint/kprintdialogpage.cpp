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

KPrintDialogPage::KPrintDialogPage(QWidget *parent)
        : QWidget(parent), m_printer(0), m_driver(0), m_ID(0), m_onlyreal(false),
        d(0)
{
}

KPrintDialogPage::KPrintDialogPage(KMPrinter *pr, DrMain *dr, QWidget *parent)
        : QWidget(parent), m_printer(pr), m_driver(dr), m_ID(0), m_onlyreal(false),
        d(0)
{
}

KPrintDialogPage::~KPrintDialogPage()
{
}

void KPrintDialogPage::setOptions(const QMap<QString, QString>&)
{
}

void KPrintDialogPage::getOptions(QMap<QString, QString>&, bool)
{
}

bool KPrintDialogPage::isValid(QString&)
{
    return true;
}

int KPrintDialogPage::id() const
{
    return m_ID;
}

void KPrintDialogPage::setId(int ID)
{
    m_ID = ID;
}

QString KPrintDialogPage::title() const
{
    return m_title;
}

void KPrintDialogPage::setTitle(const QString& txt)
{
    m_title = txt;
}

bool KPrintDialogPage::onlyRealPrinters() const
{
    return m_onlyreal;
}

void KPrintDialogPage::setOnlyRealPrinters(bool on)
{
    m_onlyreal = on;
}

DrMain* KPrintDialogPage::driver()
{
    return m_driver;
}

KMPrinter* KPrintDialogPage::printer()
{
    return m_printer;
}

#include "kprintdialogpage.moc"
