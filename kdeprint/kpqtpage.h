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

#ifndef KPQTPAGE_H
#define KPQTPAGE_H

#include "kprintdialogpage.h"

#include <kdelibs_export.h>

class QButtonGroup;
class QComboBox;
class QLabel;
class DrMain;

class KDEPRINT_EXPORT KPQtPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPQtPage(QWidget *parent = 0, const char *name = 0);
	KPQtPage(DrMain *driver, QWidget *parent = 0, const char *name = 0);
	~KPQtPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected slots:
	void slotOrientationChanged(int);
	void slotColorModeChanged(int);
	void slotNupChanged(int);

protected:
	void init();

protected:
	QButtonGroup	*m_orientbox, *m_colorbox, *m_nupbox;
	QComboBox	*m_pagesize;
	QLabel		*m_orientpix, *m_colorpix, *m_nuppix;
};

#endif
