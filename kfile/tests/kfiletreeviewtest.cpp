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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kmainwindow.h>
#include <kapp.h>
#include <kurl.h>
#include <kdebug.h>
#include <kstatusbar.h>

#include <kfiletreeview.h>

class testFrame: public KMainWindow
{
public:
   testFrame();
   void showPath( KURL & );

private:
   KFileTreeView *treeView;
};


testFrame::testFrame():KMainWindow(0,"Test FileTreeView")
{
   treeView = new KFileTreeView( this );

   /* Connect to see the status bar */
   KStatusBar* sta = statusBar();
   connect( treeView, SIGNAL( statusBarText( const QString& )),
	    sta, SLOT( message( const QString& )));
   
   
   treeView->addColumn( "Column1" );
   
   setCentralWidget( treeView );
}

void testFrame::showPath( KURL &url )
{
   QString fname = url.fileName ();
   int branchno = treeView->addBranch( url, fname );
   treeView->populateBranch( branchno );
}



int main(int argc, char **argv)
{
    KApplication a(argc, argv, "kfiletreeviewtest");
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
	  argv1 = QString::fromLatin1(argv[i]);
	  kdDebug() << "Opening " << argv1 << endl;
	  KURL u( argv1 );
	  tf->showPath( u );
       }
    }
    tf->show();
    int ret = a.exec();
    return( ret );
}
