/* This file is part of KDE desktop environment

   Copyright 2001 Michael Brade <brade@informatik.uni-muenchen.de>

   This file is licensed under LGPL version 2.
*/

#ifndef _KDIRLISTERTEST_H_
#define _KDIRLISTERTEST_H_

#include <qwidget.h>
#include <qstring.h>

#include <kurl.h>
#include <kfileitem.h>

#include <iostream>


class PrintSignals : public QObject
{
   Q_OBJECT
public:
   PrintSignals() : QObject() { }

public slots:
   void started( const KURL &url )
   {
      cout << "*** started( " << url.url().local8Bit() << " )" << endl;
   }
   void canceled() { cout << "canceled()" << endl; }
   void canceled( const KURL& url )
   {
      cout << "*** canceled( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void completed() { cout << "*** completed()" << endl; }
   void completed( const KURL& url )
   {
      cout << "*** completed( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void redirection( const KURL& url )
   {
      cout << "*** redirection( " << url.prettyURL().local8Bit() << " )" << endl;
   }
   void redirection( const KURL& src, const KURL& dest )
   {
      cout << "*** redirection( " << src.prettyURL().local8Bit() << ", "
           << dest.prettyURL().local8Bit() << " )" << endl;
   }
   void clear() { cout << "*** clear()" << endl; }
   void newItems( const KFileItemList& items )
   {
      cout << "*** newItems: " << endl;
      for ( KFileItemListIterator it( items ) ; it.current() ; ++it )
          cout << it.current()->name().local8Bit() << endl;
   }
   void deleteItem( KFileItem* item )
   {
      cout << "*** deleteItem: " << item->url().prettyURL().local8Bit() << endl;
   }
   void itemsFilteredByMime( const KFileItemList& items )
   {
      cout << "*** itemsFilteredByMime: " << endl;
      // TODO
   }
   void refreshItems( const KFileItemList& items )
   {
      cout << "*** refreshItems: " << endl;
      // TODO
   }
   void closeView() { cout << "*** closeView()" << endl; }
};

class KDirListerTest : public QWidget
{
   Q_OBJECT
public:
   KDirListerTest( QWidget *parent=0, const char *name=0 );
   ~KDirListerTest();

public slots:
   void start();
   void setURL();
   void test();

private:
   KDirLister *lister;
   PrintSignals *debug;
};

#endif
