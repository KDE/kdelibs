/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __ktypecode_h__
#define __ktypecode_h__

/**
 * A TypeCode is a code (out of the KTypeCode enum) assigned to a class type
 * To use it, call the macro K_TYPECODE( wanted_typecode ) somewhere in the
 * class definition.
 */
enum KTypeCode { TC_KService = 1, TC_KServiceType = 2, TC_KMimeType = 3,
		 TC_KFolderType = 4, TC_KDEDesktopMimeType = 5, TC_KExecMimeType = 6,
		 TC_KCustom = 1000 };

#define K_TYPECODE( c ) \
public: \
 virtual KTypeCode typeCode() const { return c; } \
private:

#endif
