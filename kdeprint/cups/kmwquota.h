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

#ifndef KMWQUOTA_H
#define KMWQUOTA_H

#include <kmwizardpage.h>

class QSpinBox;
class QComboBox;

class KMWQuota : public KMWizardPage
{
	Q_OBJECT
public:
	KMWQuota(QWidget *parent = 0, const char *name = 0);
	virtual ~KMWQuota();

	bool isValid(QString&);
	void initPrinter(KMPrinter*);
	void updatePrinter(KMPrinter*);

private:
	QSpinBox	*m_period;
	QSpinBox	*m_sizelimit;
	QSpinBox	*m_pagelimit;
	QComboBox	*m_timeunit;
};

#endif
