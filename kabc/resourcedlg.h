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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef RESOURCEDLG_H
#define RESOURCEDLG_H

#include <qmap.h>

#include <kdialog.h>
#include <klistbox.h>

#include "addressbook.h"
#include "resource.h"

namespace KABC {

/**
 * Frontend for selecting a resource.
 * 
 * Example:
 *
 * <pre>
 * KABC::ResourceDialog dlg( KABC::StdAddressBook::self(), this );
 *
 * KABC::Resource *res;
 * if ( !( res = dlg.exec() ) ) {
 *   // no resource selected
 * } else {
 *   // do something with resource
 * }
 * </pre>
 */
class ResourceDlg : KDialog
{
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param ab     The address book you want to select the resource from
   * @param parent The parent widget
   * @param name   The name of the dialog
   */
  ResourceDlg( AddressBook *ab, QWidget *parent = 0, const char *name = 0);

public slots:
  Resource *exec();

protected slots:
  void accept();

private:
  KListBox *mResourceId;

  QMap<int, Resource*> mResourceMap;
  Resource *mResource;
};

}

#endif
