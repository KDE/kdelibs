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


#ifndef KRESOURCES_RESOURCES_CONFIG_PAGE_H
#define KRESOURCES_RESOURCES_CONFIG_PAGE_H

#include <qstringlist.h>
#include <qwidget.h>

#include <kcmodule.h>

// #include "resource.h"
#include "resourcemanager.h"
class KListView;
class QPushButton;
class QListViewItem;

namespace KRES {

class Resource;

class ResourcesConfigPage : public QWidget, public ManagerListener<Resource>
{
  Q_OBJECT

public:
  ResourcesConfigPage( const QString& resourceFamily, QWidget *parent = 0, const char *name = 0 );
  virtual ~ResourcesConfigPage();

  void load();
  void save();
  virtual void defaults();

public slots:
  void slotAdd();
  void slotRemove();
  void slotEdit();
  void slotStandard();
  void slotSelectionChanged();

  // From ManagerListener<Resource>
public:
  virtual void resourceAdded( Resource* resource );
  virtual void resourceModified( Resource* resource );
  virtual void resourceDeleted( Resource* resource );

signals:
  void changed(bool);

private:
  ResourceManager<Resource>* mManager;
  QString mFamily;

  KListView* mListView;
  QPushButton* mAddButton;
  QPushButton* mRemoveButton;
  QPushButton* mEditButton;
  QPushButton* mStandardButton;

  QListViewItem* mLastItem;
};

}

#endif
