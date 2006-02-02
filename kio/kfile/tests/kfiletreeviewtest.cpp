/* This file is part of the KDE libraries
    Copyright (C) 2001 Klaas Freitag <freitag@suse.de>

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

#include <qdir.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kmainwindow.h>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kstatusbar.h>

#include <kfiletreeview.h>
#include "kfiletreeviewtest.h"


#include "kfiletreeviewtest.moc"

testFrame::testFrame():KMainWindow(0,"Test FileTreeView"),
		       dirOnlyMode(false)

{
   treeView = new KFileTreeView( this );
   treeView->setDragEnabled( true );
   treeView->setAcceptDrops( true );
   treeView->setDropVisualizer( true );


   /* Connect to see the status bar */
   KStatusBar* sta = statusBar();
   connect( treeView, SIGNAL( onItem( const QString& )),
	    sta, SLOT( message( const QString& )));

   connect( treeView, SIGNAL( dropped( QWidget*, QDropEvent*, KUrl::List& )),
	    this, SLOT( urlsDropped( QWidget*, QDropEvent*, KUrl::List& )));

   connect( treeView, SIGNAL( dropped( KUrl::List&, KUrl& )), this,
	    SLOT( copyURLs( KUrl::List&, KUrl& )));

   treeView->addColumn( "File" );
   treeView->addColumn( "ChildCount" );
   setCentralWidget( treeView );
   resize( 600, 400 );

   showPath( KUrl::fromPathOrURL( QDir::homePath() ));
}

void testFrame::showPath( const KUrl &url )
{
   QString fname = "TestBranch"; // url.fileName ();
   /* try a user icon */
   KIconLoader *loader = KGlobal::iconLoader();
   QPixmap pix = loader->loadIcon( "contents2", KIcon::Small );
   QPixmap pixOpen = loader->loadIcon( "contents", KIcon::Small );

   KFileTreeBranch *nb = treeView->addBranch( url, fname, pix );

   if( nb )
   {
      if( dirOnlyMode ) treeView->setDirOnlyMode( nb, true );
      nb->setOpenPixmap( pixOpen );

      connect( nb, SIGNAL(populateFinished(KFileTreeViewItem*)),
	       this, SLOT(slotPopulateFinished(KFileTreeViewItem*)));
      connect( nb, SIGNAL( directoryChildCount( KFileTreeViewItem *, int )),
	       this, SLOT( slotSetChildCount( KFileTreeViewItem*, int )));
      // nb->setChildRecurse(false );

      nb->setOpen(true);
   }


}

void testFrame::urlsDropped( QWidget* , QDropEvent* , KUrl::List& list )
{
   KUrl::List::ConstIterator it = list.begin();
   for ( ; it != list.end(); ++it ) {
      kDebug() << "Url dropped: " << (*it).prettyURL() << endl;
   }
}

void testFrame::copyURLs( KUrl::List& list, KUrl& to )
{
   KUrl::List::ConstIterator it = list.begin();
   kDebug() << "Copy to " << to.prettyURL() << endl;
   for ( ; it != list.end(); ++it ) {
      kDebug() << "Url: " << (*it).prettyURL() << endl;
   }

}


void testFrame::slotPopulateFinished(KFileTreeViewItem *item )
{
   if( item )
   {
#if 0
      int cc = item->childCount();

      kDebug() << "setting column 2 of treeview with count " << cc << endl;

      item->setText( 1, QString::number( cc ));
#endif
   }
   else
   {
      kDebug() << "slotPopFinished for uninitalised item" << endl;
   }
}

void testFrame::slotSetChildCount( KFileTreeViewItem *item, int c )
{
   if( item )
      item->setText(1, QString::number( c ));
}

int main(int argc, char **argv)
{
    KApplication a;
    QString name1;
    QStringList names;

    QString argv1;
    testFrame *tf;

    tf =  new testFrame();
    a.setMainWidget( tf );

    if (argc > 1)
    {
       for( int i = 1; i < argc; i++ )
       {
	  argv1 = QLatin1String(argv[i]);
	  kDebug() << "Opening " << argv1 << endl;
	  if( argv1 == "-d" )
	     tf->setDirOnly();
	  else
	  {
	  KUrl u( argv1 );
	  tf->showPath( u );
       }
    }
    }
    tf->show();
    int ret = a.exec();
    return( ret );
}
