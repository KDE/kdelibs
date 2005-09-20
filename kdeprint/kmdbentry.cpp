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

#include "kmdbentry.h"
#include "kmfactory.h"
#include "kmmanager.h"

KMDBEntry::KMDBEntry() : recommended(false)
{
}

bool KMDBEntry::validate(bool checkIt)
{
	// check model: if one of "model" or "modelname" is empty, replace
	// by the other one. At the end, "model" must be non empty.
	if (model.isEmpty())
	{
		model = modelname;
	}
	if (modelname.isEmpty())
	{
		modelname = model;
	}
	if (model.isEmpty())
		return false;

	// check manufacturer
	if (manufacturer.isEmpty())
	{
		int	p = model.find(' ',0);
		if (p != -1) manufacturer = model.left(p);
		if (manufacturer.isEmpty()) return false;
	}

	// extract manufacturer from model
	if (model.find(manufacturer,0,false) == 0)
	{
		model = model.right(model.length()-manufacturer.length()-1).trimmed();
		if (model.isEmpty()) return false;
	}

	if (checkIt)
		return KMFactory::self()->manager()->validateDbDriver(this);
	else
		return true;
}
