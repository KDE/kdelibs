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

#include <qdir.h>

#include "kfmpaths.h"
#include <ksimpleconfig.h>
#include <kapp.h>
#include <qstring.h>

QString* KfmPaths::s_desktopPath = 0;
QString* KfmPaths::s_templatesPath = 0;
QString* KfmPaths::s_autostartPath = 0;
QString* KfmPaths::s_trashPath = 0;

void KfmPaths::initStatic() 
{
  if ( s_desktopPath != 0 )
    return;
  
  s_desktopPath = new QString();
  s_templatesPath = new QString();
  s_autostartPath = new QString();
  s_trashPath = new QString();

  KSimpleConfig *config = new KSimpleConfig(kapp->localconfigdir()+"/kfmrc", true);
  config->setGroup( "Paths" );

  // Desktop Path
  *s_desktopPath = QDir::homeDirPath() + "/Desktop/";
  *s_desktopPath = config->readEntry( "Desktop", *s_desktopPath);
  if ( s_desktopPath->right(1) != "/")
    *s_desktopPath += "/";
  
  // Templates Path
  *s_templatesPath = *s_desktopPath + "Templates/";
  *s_templatesPath = config->readEntry( "Templates" , *s_templatesPath);
  if ( s_templatesPath->right(1) != "/")
    *s_templatesPath += "/";

  // Autostart Path
  *s_autostartPath = *s_desktopPath + "Autostart/";
  *s_autostartPath = config->readEntry( "Autostart" , *s_autostartPath);
  if ( s_autostartPath->right(1) != "/")
    *s_autostartPath += "/";

  // Trash Path
  *s_trashPath = *s_desktopPath + "Trash/";
  *s_trashPath = config->readEntry( "Trash" , *s_trashPath);
  if ( s_autostartPath->right(1) != "/")
    *s_autostartPath += "/";
}
