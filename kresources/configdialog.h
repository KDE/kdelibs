/*
    This file is part of libkresources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#ifndef KRESOURCES_RESOURCECONFIGDLG_H
#define KRESOURCES_RESOURCECONFIGDLG_H

#include <kdialog.h>

// #include "resource.h"
// #include "resourceconfigwidget.h"
class KLineEdit;
class QCheckBox;
class KButtonBox;
namespace KRES {
  class Resource;
  class ResourceConfigWidget;

class ResourceConfigDlg : KDialog
{
  Q_OBJECT

public:
  // Resource=0: create new resource
  ResourceConfigDlg( QWidget *parent, const QString& resourceFamily,
	  /*const QString& type,*/ Resource* resource, /*KConfig *config,*/ const char *name = 0);

public slots:
  int exec();

protected slots:
  void accept();
  void setReadOnly( bool value );
    void slotNameChanged( const QString &text);
private:
  ResourceConfigWidget *mConfigWidget;
  Resource* mResource;

  KButtonBox *mButtonBox;
  KLineEdit *mName;
  QCheckBox *mReadOnly;
    QPushButton *mbuttonOk;
};

}

#endif
