/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kdirnotify.h"

#include <q3cstring.h>

// Needed since DCOP enforces object id uniqueness.
int KDirNotify::s_serial = 0;

KDirNotify::KDirNotify()
  :  DCOPObject( Q3CString().sprintf("KDirNotify-%d", ++s_serial) )
{
   connectDCOPSignal(0, "KDirNotify", "FilesAdded(KURL)", "FilesAdded(KURL)", false);
   connectDCOPSignal(0, "KDirNotify", "FilesRemoved(KURL::List)", "FilesRemoved(KURL::List)", false);
   connectDCOPSignal(0, "KDirNotify", "FilesChanged(KURL::List)", "FilesChanged(KURL::List)", false);
   connectDCOPSignal(0, "KDirNotify", "FileRenamed(KURL,KURL)", "FileRenamed(KURL,KURL)", false);
}

void KDirNotify::FileRenamed( const KURL &, const KURL & )
{
}

void KDirNotify::virtual_hook( int id, void* data )
{ DCOPObject::virtual_hook( id, data ); }

