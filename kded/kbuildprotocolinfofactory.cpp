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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kbuildprotocolinfofactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

KBuildProtocolInfoFactory::KBuildProtocolInfoFactory() :
  KProtocolInfoFactory()
{
   m_resourceList = new KSycocaResourceList();
   m_resourceList->add( "services", "*.protocol" );
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildProtocolInfoFactory::resourceTypes()
{
    return QStringList() << "services";
}

KBuildProtocolInfoFactory::~KBuildProtocolInfoFactory()
{
   delete m_resourceList;
}

KProtocolInfo *
KBuildProtocolInfoFactory::createEntry( const QString& file, const char * )
{
   return new KProtocolInfo(file);
}
