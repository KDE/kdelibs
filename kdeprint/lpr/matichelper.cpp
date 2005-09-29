/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <kdeprint@swing.be>
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

#include "matichelper.h"
#include "printcapentry.h"
#include "driver.h"

QString maticFile(PrintcapEntry *entry)
{
	QString	s(entry->field("af"));
	if (s.isEmpty())
	{
		s = entry->field("filter_options");
		if (!s.isEmpty())
		{
			int	p = s.findRev(' ');
			if (p != -1)
				s = s.mid(p+1);
		}
	}
	return s;
}
