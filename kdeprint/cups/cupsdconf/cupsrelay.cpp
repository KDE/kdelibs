/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#include "cupsrelay.h"

#include <klocale.h>

CupsRelay::CupsRelay(QWidget *parent, const char *name)
	: CupsListBox(2,parent,name)
{
	setColumnText(0, i18n("From"));
	setColumnText(1, i18n("To"));
	setAddMessage(i18n("From"), 0);
	setAddMessage(i18n("To"), 1);
}

CupsRelay::~CupsRelay()
{
}

QString CupsRelay::relayFrom(int index)
{
	return text(index, 0);
}

QString CupsRelay::relayTo(int index)
{
	return text(index, 1);
}

void CupsRelay::addRelay(const QString& from, const QString& to)
{
	QStringList	list;
	list.append(from);
	list.append(to);
	insertItem(list);
}
