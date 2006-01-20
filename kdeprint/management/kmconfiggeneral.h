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

#ifndef KMCONFIGGENERAL_H
#define KMCONFIGGENERAL_H

#include "kmconfigpage.h"

class KIntNumInput;
class KUrlRequester;
class QCheckBox;
class QPushButton;
class KMConfigGeneral : public KMConfigPage
{
	Q_OBJECT
public:
	KMConfigGeneral(QWidget *parent = 0);

	void loadConfig(KConfig*);
	void saveConfig(KConfig*);

protected Q_SLOTS:
	void slotTestPagePreview();
        void testPageChanged(const QString & );
	void setEnabledPreviewButton(bool b);
private:
	KIntNumInput	*m_timer;
	KUrlRequester	*m_testpage;
	QCheckBox	*m_defaulttestpage;
	QPushButton	*m_preview;
	QCheckBox	*m_statusmsg, *m_uselast;
};

#endif
