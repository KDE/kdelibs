/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KMCOP_H
#define KMCOP_H

#include <qobject.h>

#include <dcopobject.h>

class KMCOPPrivate;

class KMCOP : public QObject,
			  public DCOPObject
{
Q_OBJECT
K_DCOP

public:
	KMCOP();
	~KMCOP();
	
k_dcop:
	int objectCount();
	void addInterfacesHackHackHack();

private:
	Q3CString correctType(const Q3CString &str);
    KMCOPPrivate *d;
};


#endif

