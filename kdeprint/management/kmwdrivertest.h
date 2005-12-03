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

#ifndef KMWDRIVERTEST_H
#define KMWDRIVERTEST_H

#include "kmwizardpage.h"

class QLabel;
class QPushButton;
class DrMain;

class KMWDriverTest : public KMWizardPage
{
	Q_OBJECT
public:
	KMWDriverTest(QWidget *parent = 0);
	~KMWDriverTest();

	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

protected slots:
	void slotTest();
	void slotSettings();

private:
	QLabel	*m_manufacturer;
	QLabel	*m_model;
	QLabel	*m_driverinfo;
	QPushButton	*m_test;
	QPushButton	*m_settings;
	DrMain		*m_driver;
	KMPrinter	*m_printer;
};

#endif
