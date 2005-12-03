/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__

#include <sys/stat.h>

#include <qobject.h>
#include <qstring.h>

#include <kservice.h>
#include <ksycoca.h>
#include <ksycocatype.h>
#include <ksycocaentry.h>
#include <kservicegroup.h>

#include "vfolder_menu.h"

class QDataStream;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
   Q_OBJECT
public:
   KBuildSycoca();
   virtual ~KBuildSycoca();

   /**
    * Recreate the database file
    */
   bool recreate();

   static bool checkTimestamps( quint32 timestamp, const QStringList &dirs );

   static QStringList existingResourceDirs();

   void setTrackId(const QString &id) { m_trackId = id; }

protected slots:
   void slotCreateEntry(const QString &file, KService::Ptr *entry);

protected:

   /**
    * Look up gnome mimetypes.
    */
   void processGnomeVfs();

   /**
    * Add single entry to the sycoca database.
    * Either from a previous database or regenerated from file.
    */
   KSycocaEntry::Ptr createEntry(const QString &file, bool addToFactory);

   /**
    * Convert a VFolderMenu::SubMenu to KServiceGroups.
    */
   void createMenu(QString caption, QString name, VFolderMenu::SubMenu *menu);

   /**
    * Build the whole system cache, from .desktop files
    */
   bool build();

   /**
    * Save the ksycoca file
    */
   void save();

   /**
    * Clear the factories
    */
   void clear();

   static bool checkDirTimestamps( const QString& dir, const QDateTime& stamp, bool top );

   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return true; }

   void createMenuAttribute( KServiceGroup::Ptr entry );

   QStringList m_allResourceDirs;
   QString m_trackId;
};

#endif
