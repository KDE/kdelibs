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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KICONSELECTACTION_H
#define KICONSELECTACTION_H

#include <kaction.h>

class KIconSelectActionPrivate;

class KIconSelectAction : public KSelectAction
{
	Q_OBJECT
public:
	KIconSelectAction(const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0);
	virtual ~KIconSelectAction();

	virtual int plug(QWidget* widget, int index = -1);

public slots:
	virtual void setItems(const QStringList& lst, const QStringList& iconlst);
	virtual void setCurrentItem(int);

protected:
	void createPopupMenu();
	void updateIcons();
	virtual void updateCurrentItem(int id);

private:
	KIconSelectActionPrivate*	d;
};

#endif
