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

#include <kvisiblebuttongroup.h>
#include <kseparator.h>
#include <kdialog.h>
#include <kcursor.h>

#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

KVisibleButtonGroup::KVisibleButtonGroup(QWidget *parent)
    : QGroupBox(parent), QButtonGroup(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(KDialog::spacingHint());
	m_layout->addStretch(1);
	m_count = 0;
}

KVisibleButtonGroup::~KVisibleButtonGroup()
{
}

int KVisibleButtonGroup::addButton(QAbstractButton *btn, int ID)
{
	if(!btn)
	{
		KSeparator* sep = new KSeparator( Qt::Horizontal, this );
		m_layout->insertWidget(m_count, sep);
	}
	else
	{
		if(ID == KVisibleButtonGroup::nextIndex)
			dynamic_cast<QButtonGroup *>(this)->addButton(btn);
		else
			dynamic_cast<QButtonGroup *>(this)->addButton(btn, ID);
		m_layout->insertWidget(m_count, btn);
	}
	m_count++;

	return (m_count - 1);
}

KRadioButtonWithHandOver::KRadioButtonWithHandOver(const QString& txt, QWidget *parent)
: QRadioButton(txt,parent)
{
	setCursor(KCursor::handCursor());
}

