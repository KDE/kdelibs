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

#ifndef KPIMAGEPAGE_H
#define KPIMAGEPAGE_H

#include "kprintdialogpage.h"

class KIntNumInput;
class QComboBox;
class Q3ButtonGroup;
class ImagePreview;
class ImagePosition;

class KPImagePage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPImagePage(DrMain *driver = 0, QWidget *parent = 0);
	~KPImagePage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected Q_SLOTS:
	void slotSizeTypeChanged(int);
	void slotPositionChanged();
	void slotImageSettingsChanged();
	void slotDefaultClicked();

private:
	KIntNumInput	*m_brightness, *m_hue, *m_saturation, *m_gamma;
	QComboBox		*m_sizetype;
	KIntNumInput	*m_size;
	Q3ButtonGroup	*m_vertgrp, *m_horizgrp;
	ImagePreview	*m_preview;
	ImagePosition	*m_position;
};

#endif
