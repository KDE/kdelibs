/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *            (C) 1999 Waldo Bastian <bastian@kde.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kded_h__
#define __kded_h__ 

#include <qobject.h>
#include <qstring.h>
#include <qtimer.h>
#include <qasciidict.h>
#include <qintdict.h>

#include <dcopclient.h>
#include <dcopobject.h>

#include <ksycoca.h>
#include <ksycocatype.h>
#include <kdedmodule.h>
#include <klibloader.h>

class KDirWatch;
class KService;

// No need for this in libkio - apps only get readonly access
class Kded : public QObject, public DCOPObject, public DCOPObjectProxy
{
  Q_OBJECT
public:
   Kded(bool checkUpdates);
   virtual ~Kded();

   static Kded *self() { return _self;}
   /**
    * Catch calls to unknown objects.
    */
   bool process(const QCString &obj, const QCString &fun, 
                const QByteArray &data, 
		QCString &replyType, QByteArray &replyData);

   /**
    * process DCOP message.  Only calls to "recreate" are supported at
    * this time.
    */
   bool process(const QCString &fun, const QByteArray &data, 
		QCString &replyType, QByteArray &replyData);

   virtual QCStringList functions();

   void noDemandLoad(const QString &obj); // Don't load obj on demand

   KDEDModule *loadModule(const QCString &obj, bool onDemand);
   KDEDModule *loadModule(const KService *service, bool onDemand);
   QCStringList loadedModules();
   bool unloadModule(const QCString &obj);
   bool isWindowRegistered(long windowId);
   void registerWindowId(long windowId);
   void unregisterWindowId(long windowId);

public slots:

   /**
    * Recreate the database file
    */
   void recreate();

   /**
    * Collect all directories to watch
    */
   void build();

   /**
    * An application unregistered itself with DCOP
    */
   void slotApplicationRemoved(const QCString &appId);

   /**
    * A KDEDModule is about to get destroyed.
    */
   void slotKDEDModuleRemoved(KDEDModule *);

protected slots:

   /**
    * @internal Triggers rebuilding
    */
   void dirDeleted(const QString& path);
 
   /**
    * @internal Triggers rebuilding
    */
   void update (const QString& dir );

   /**
    * @internal Installs crash handler
    */
   void installCrashHandler();

protected:


   /**
    * Scans dir for new files and new subdirectories.
    */
   void readDirectory(const QString& dir );

   static void crashHandler(int);
   
protected:

   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    * Slower polling for remote file systems is now done in KDirWatch (JW).
    */
   KDirWatch* m_pDirWatch;

   bool b_checkUpdates;

   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   QTimer* m_pTimer;
   
   QValueList<DCOPClientTransaction *> m_requests;
   QAsciiDict<KDEDModule> m_modules;
   QAsciiDict<KLibrary> m_libs;
   QAsciiDict<QObject> m_dontLoad;
   QAsciiDict<QValueList<long> > m_windowIdList;
   QIntDict<long> m_globalWindowIdList;
     
   static Kded *_self;
};

class KUpdateD : public QObject
{
   Q_OBJECT
public:
   KUpdateD();
   ~KUpdateD();
   
public slots:
   void runKonfUpdate();
   void slotNewUpdateFile();

private:
   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    * Slower polling for remote file systems is now done in KDirWatch (JW).
    */
   KDirWatch* m_pDirWatch;

   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   QTimer* m_pTimer;
};

class KHostnameD : public QObject
{
   Q_OBJECT
public:
   KHostnameD(int pollInterval);
   ~KHostnameD();
   
public slots:
   void checkHostname();

private:
   /**
    * Timer for interval hostname checking.
    */
   QTimer m_Timer;
   QCString m_hostname;
};

#endif
