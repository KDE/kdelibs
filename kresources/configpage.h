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


#ifndef KRESOURCES_CONFIGPAGE_H
#define KRESOURCES_CONFIGPAGE_H

#include <qstringlist.h>
#include <qwidget.h>

#include <kcmodule.h>

#include "manager.h"

class KListView;
class QPushButton;
class QListViewItem;

namespace KRES {

class Resource;

class ConfigPage : public QWidget, public ManagerListener<Resource>
{
  Q_OBJECT

  public:
    ConfigPage( const QString &family, const QString &config = QString::null,
                QWidget *parent = 0, const char *name = 0 );
    ConfigPage( const QString &family, QWidget *parent = 0,
                const char *name = 0 );
    virtual ~ConfigPage();

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

  protected slots:
    void slotItemClicked( QListViewItem * );

  signals:
    void changed( bool );

  private:
    void init( const QString &configFile );

    Manager<Resource>* mManager;
    QString mFamily;
    KConfig *mConfig;

    KListView* mListView;
    QPushButton* mAddButton;
    QPushButton* mRemoveButton;
    QPushButton* mEditButton;
    QPushButton* mStandardButton;

    QListViewItem* mLastItem;
};

}

#endif
