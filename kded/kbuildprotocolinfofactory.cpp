/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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

#include "kbuildprotocolinfofactory.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocaresourcelist.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <assert.h>

KBuildProtocolInfoFactory::KBuildProtocolInfoFactory() :
  KProtocolInfoFactory()
{
   m_resourceList = new KSycocaResourceList();
   m_resourceList->add("services", "kde5/services", "*.protocol");
}

// return all resource dirs for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildProtocolInfoFactory::resourceDirs()
{
    return QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kde5/services", QStandardPaths::LocateDirectory);
}

KBuildProtocolInfoFactory::~KBuildProtocolInfoFactory()
{
   delete m_resourceList;
}

KProtocolInfo *KBuildProtocolInfoFactory::createEntry(const QString& file) const
{
   return new KProtocolInfo(file);
}
