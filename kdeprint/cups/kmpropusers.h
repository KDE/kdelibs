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

#ifndef KMPROPUSERS_H
#define KMPROPUSERS_H

#include "kmpropwidget.h"

class QTextView;

class KMPropUsers : public KMPropWidget
{
public:
	KMPropUsers(QWidget *parent = 0, const char *name = 0);
	~KMPropUsers();

	void setPrinter(KMPrinter*);

protected:
	void configureWizard(KMWizard*);

private:
	QTextView	*m_text;
};

#endif
