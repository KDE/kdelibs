/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2006 Alexander Kern <alex.kern@gmx.de>
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

#ifndef KVISIBLEBUTTONGROUP_H
#define KVISIBLEBUTTONGROUP_H


#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>


class KVisibleButtonGroup : public QGroupBox, public QButtonGroup
{
public:
	enum Index {
		nextIndex = -1
	};

public:
	KVisibleButtonGroup(QWidget *parent = 0);
	~KVisibleButtonGroup();

	int addButton(QAbstractButton *btn, int ID = KVisibleButtonGroup::nextIndex);

private:
	QVBoxLayout	*m_layout;
	int 		m_count;
};

class KRadioButtonWithHandOver : public QRadioButton
{
public:
	KRadioButtonWithHandOver(const QString& txt, QWidget *parent = 0);
};

#endif
