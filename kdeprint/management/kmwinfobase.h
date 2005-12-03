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

#ifndef KMWINFOBASE_H
#define KMWINFOBASE_H

#include "kmwizardpage.h"
#include <q3ptrlist.h>

class QLabel;
class QLineEdit;

class KDEPRINT_EXPORT KMWInfoBase : public KMWizardPage
{
public:
	KMWInfoBase(int n = 1, QWidget *parent = 0);

	void setInfo(const QString&);
	void setLabel(int, const QString&);
	void setText(int, const QString&);
	void setCurrent(int);

	QString text(int);

protected:
	QLineEdit* lineEdit( int );

private:
	Q3PtrList<QLabel>		m_labels;
	Q3PtrList<QLineEdit>	m_edits;
	QLabel			*m_info;
	int			m_nlines;
};

#endif
