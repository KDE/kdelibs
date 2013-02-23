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
#ifndef KDED_KBUILDSYCOCA_H
#define KDED_KBUILDSYCOCA_H

#include "kbuildsycocainterface.h"

#include <sys/stat.h>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kservice.h>
#include <ksycoca.h>
#include <ksycocatype.h>
#include <ksycocaentry.h>
#include <kservicegroup.h>

#include "vfolder_menu.h"

class QDataStream;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca, public KBuildSycocaInterface
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

   QStringList changedResources() const { return m_changedResources; }

    // Use our friendly-access-to-KSycoca to make this public
    static void clearCaches() { KSycoca::clearCaches(); }
    QString kfsstnd_prefixes() { return KSycoca::kfsstnd_prefixes(); }

    /**
     * Returns a number that identifies the current version of the file @p filename,
     * which is located under GenericDataLocation (including local overrides).
     *
     * When a change is made to the file this number will change.
     */
    static quint32 calcResourceHash(const QString& subdir, const QString &filename);

private:
   /**
    * Add single entry to the sycoca database.
    * Either from a previous database or regenerated from file.
    */
   KSycocaEntry::Ptr createEntry(const QString &file, bool addToFactory);

    /**
     * Implementation of KBuildSycocaInterface
     * Create service and return it. The caller must add it to the servicefactory.
     */
    /*! \reimp */ KService::Ptr createService(const QString& path);

   /**
    * Convert a VFolderMenu::SubMenu to KServiceGroups.
    */
   void createMenu(const QString &caption, const QString &name, VFolderMenu::SubMenu *menu);

   /**
    * Build the whole system cache, from .desktop files
    */
   bool build();

   /**
    * Save the ksycoca file
    */
   void save(QDataStream* str);

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

   QStringList m_changedResources;
   QStringList m_allResourceDirs;
   QString m_trackId;

    QByteArray g_resource; // e.g. "services" (old resource name, now only used for the signal, see kctimefactory.cpp)
    QString g_resourceSubdir; // e.g. "kde5/services" (xdgdata subdir)
};

#endif
