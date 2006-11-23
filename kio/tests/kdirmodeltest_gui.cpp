/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
 */

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdirlister.h>
#include <kdirmodel.h>
#include <kurl.h>

#include <QTreeView>
#include <QListView>
#include <QDir>
#include <QFile>
#include <kmimetyperesolver.h>

//#include "kdirmodeltest_gui.h"

static const KCmdLineOptions options[] =
{
  {"+[directory ...]", "Directory(ies) to model", 0},
  KCmdLineLastOption
};


int main (int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "kdirmodeltest", "KDirModelTest",
		     "Test for KDirModel", "1.0");
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::addStdCmdLineOptions();

  KApplication a;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KDirModel *dirmodel = new KDirModel(0);
  dirmodel->dirLister()->setDelayedMimeTypes(true);

#if 1
  QTreeView* treeView = new QTreeView(0);
  treeView->setModel(dirmodel);
  treeView->setUniformRowHeights(true); // makes visualRect() much faster
  treeView->resize(500, 500);
  treeView->show();
  new KMimeTypeResolver(treeView, dirmodel);
#endif

#if 0
  QListView* listView = new QListView(0);
  listView->setModel(dirmodel);
  listView->setUniformItemSizes(true); // true in list mode, not in icon mode.
  listView->show();
  new KMimeTypeResolver(listView, dirmodel);
#endif

#if 1
  QListView* iconView = new QListView(0);
  iconView->setModel(dirmodel);
  iconView->setSelectionMode(QListView::ExtendedSelection);
  iconView->setViewMode(QListView::IconMode);
  iconView->show();
  new KMimeTypeResolver(iconView, dirmodel);
#endif

  if (args->count() == 0)
      dirmodel->dirLister()->openUrl( QDir::homePath() );

  for(int i = 0; i < args->count(); i++) {
      kDebug() << "Adding: " << args->url(i) << endl;
      dirmodel->dirLister()->openUrl( args->url(i), true /*keep*/ );
  }

  return a.exec();
}
