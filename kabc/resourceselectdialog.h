/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCESELECTDIALOG_H
#define KABC_RESOURCESELECTDIALOG_H

#include <qmap.h>

#include <kdialog.h>
#include <kdemacros.h>

class KListBox;

namespace KABC {

class AddressBook;
class Resource;

/**
  This class is @deprecated, use KRES::SelectDialog instead. 
 */
class KABC_EXPORT_DEPRECATED ResourceSelectDialog : KDialog
{
  Q_OBJECT

  public:
    ResourceSelectDialog( AddressBook *ab, QWidget *parent = 0,
                          const char *name = 0);
    Resource *resource();

    static Resource *getResource( AddressBook *ab, QWidget *parent = 0 );

  private:
    KListBox *mResourceId;
    QMap<int, Resource*> mResourceMap;
};

}

#endif
