/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kio/observer.h>
#include <kdebug.h>

using namespace KIO;

Observer * Observer::s_pObserver = 0L;

Observer::Observer()
{
}

void Observer::slotTotalEntries( KIO::Job* job, unsigned long count )
{
  kdDebug() << "** Observer::slotTotalEntries " << job << " " << count << endl;
}

void Observer::slotEntries( KIO::Job* job, const KIO::UDSEntryList& entries )
{
  kdDebug() << "** Observer::slotEntries " << job << " " << entries.count() << endl;
}
