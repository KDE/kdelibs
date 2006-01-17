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

#ifndef KMPROPCONTAINER_H
#define KMPROPCONTAINER_H

#include <qwidget.h>

class KMPropWidget;
class QPushButton;
class KMPrinter;

class KMPropContainer : public QWidget
{
	Q_OBJECT
public:
	KMPropContainer(QWidget *parent = 0);
	~KMPropContainer();

	void setWidget(KMPropWidget*);
	void setPrinter(KMPrinter*);

Q_SIGNALS:
	void enable(bool);

protected Q_SLOTS:
	void slotEnableChange(bool);

private:
	KMPropWidget	*m_widget;
	QPushButton	*m_button;
};

#endif
