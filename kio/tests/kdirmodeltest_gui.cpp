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

#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <kmimetyperesolver.h>
#include <kfileitemdelegate.h>

// Test controller for making the view open up while expandToUrl lists subdirs
class TreeController : public QObject
{
    Q_OBJECT
public:
    explicit TreeController(QTreeView* view, KDirModel* model)
        : QObject(view), m_treeView(view), m_model(model)
    {
      connect(model, SIGNAL(expand(QModelIndex)),
              this, SLOT(slotExpand(QModelIndex)));
    }
private Q_SLOTS:
    void slotExpand(const QModelIndex& index)
    {
        KFileItem item = m_model->itemForIndex(index);
        kDebug() << "slotListingCompleted" << item.url();
        m_treeView->setExpanded(index, true);

        // The scrollTo call doesn't seem to work.
        // We probably need to delay this until everything's listed and layouted...
        m_treeView->scrollTo(index);
    }
private:
    QTreeView* m_treeView;
    KDirModel* m_model;
};

int main (int argc, char **argv)
{
  KCmdLineOptions options;
  options.add("+[directory ...]", ki18n("Directory(ies) to model"));

  KCmdLineArgs::init(argc, argv, "kdirmodeltest", 0, ki18n("KDirModelTest"),
		     "1.0", ki18n("Test for KDirModel"));
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
  treeView->setItemDelegate( new KFileItemDelegate(treeView) );
#endif

#if 0
  QListView* listView = new QListView(0);
  listView->setModel(dirmodel);
  listView->setUniformItemSizes(true); // true in list mode, not in icon mode.
  listView->show();
  new KMimeTypeResolver(listView, dirmodel);
  listView->setItemDelegate( new KFileItemDelegate(listView) );
#endif

#if 1
  QListView* iconView = new QListView(0);
  iconView->setModel(dirmodel);
  iconView->setSelectionMode(QListView::ExtendedSelection);
  iconView->setViewMode(QListView::IconMode);
  iconView->show();
  new KMimeTypeResolver(iconView, dirmodel);
  iconView->setItemDelegate( new KFileItemDelegate(iconView) );
#endif

  if (args->count() == 0) {
      dirmodel->dirLister()->openUrl(KUrl("/"));

      const KUrl url("/usr/share/applications/kde");
      dirmodel->expandToUrl(url);
      new TreeController(treeView, dirmodel);
  }

  for(int i = 0; i < args->count(); i++) {
      kDebug() << "Adding: " << args->url(i);
      dirmodel->dirLister()->openUrl( args->url(i), KDirLister::Keep );
  }

  return a.exec();
}

#include "kdirmodeltest_gui.moc"
