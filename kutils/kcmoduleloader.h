/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  
  This file is part of the KDE project
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/
#ifndef MODLOADER_H
#define MODLOADER_H

#include <kcmodule.h>
#include <kcmoduleinfo.h>

class QWidget;
class KLibLoader;

class KCModuleLoader
{
  /**
   * @description loads a KControl Module
   * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
   * @since 3.2
   * @internal
   **/
public:
  static KCModule *loadModule(const KCModuleInfo &mod, bool withfallback=true, QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );
  static KCModule *loadModule(const QString &module, QWidget *parent = 0,
      const char *name = 0, const QStringList & args = QStringList());
  static void unloadModule(const KCModuleInfo &mod);
  static void showLastLoaderError(QWidget *parent);
private:
  static KCModule* load(const KCModuleInfo &mod, const QString &libname, KLibLoader *loader, QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );

};

#endif // MODLOADER_H

