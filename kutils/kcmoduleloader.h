/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
  static KCModule *loadModule(const KCModuleInfo &mod, bool withfallback=true);
  static void unloadModule(const KCModuleInfo &mod);
  static void showLastLoaderError(QWidget *parent);
private:
  static KCModule* load(const KCModuleInfo &mod, const QString &libname, KLibLoader *loader);

};

#endif // MODLOADER_H

