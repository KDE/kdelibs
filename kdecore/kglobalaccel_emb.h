/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KGLOBALACCEL_EMB_H
#define KGLOBALACCEL_EMB_H

#include "kaccelbase.h"
#include "kshortcut.h"

class KGlobalAccelPrivate
{
public:
	KGlobalAccelPrivate();
	virtual ~KGlobalAccelPrivate(){};
	
	virtual void setEnabled( bool );

	virtual bool connectKey( KAccelAction&, KKeySequence );
	virtual bool disconnectKey( KAccelAction&, KKeySequence );
};

#endif // KGLOBALACCEL_EMB_H

