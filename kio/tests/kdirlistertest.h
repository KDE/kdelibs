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
   void started( const QString &url )
   {
      cout << "*** started( " << url.latin1() << " )" << endl;
   }
   void canceled() { cout << "canceled()" << endl; }
   void canceled( const KURL& url )
   {
      cout << "*** canceled( " << url.prettyURL().latin1() << " )" << endl;
   }
   void completed() { cout << "*** completed()" << endl; }
   void completed( const KURL& url )
   {
      cout << "*** completed( " << url.prettyURL().latin1() << " )" << endl;
   }
   void redirection( const KURL& url )
   {
      cout << "*** redirection( " << url.prettyURL().latin1() << " )" << endl;
   }
   void redirection( const KURL& src, const KURL& dest )
   {
      cout << "*** redirection( " << src.prettyURL().latin1() << ", "
           << dest.prettyURL().latin1() << " )" << endl;
   }
   void clear() { cout << "*** clear()" << endl; }
   void newItems( const KFileItemList& items )
   {
      cout << "*** newItems: " << endl;
      // TODO
   }
   void deleteItem( KFileItem* item )
   {
      cout << "*** deleteItem: " << item->url().prettyURL().latin1() << endl;
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
