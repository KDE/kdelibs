/*
This file is part of KDE

  Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
//----------------------------------------------------------------------------
//
// KDE Cookie Server
// $Id$

#define SAVE_DELAY 3 // Save after 3 minutes

#include "kcookieserver.h"
#include "kcookiejar.h"
#include "kcookiewin.h"

#include <kdebug.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kstddirs.h>
#include <qtimer.h>
#include <unistd.h>
#include <qlist.h>

#include <dcopclient.h>

class CookieRequest {
public:
   DCOPClientTransaction *transaction;
   QString url;
};

template class  QList<CookieRequest>;

class RequestList : public QList<CookieRequest>
{
public:
   RequestList() : QList<CookieRequest>() { }
};

KCookieServer::KCookieServer()
  : KUniqueApplication()
{
   mCookieJar = new KCookieJar;
   mPendingCookies = new KHttpCookieList;
   mRequestList = new RequestList;
   mAdvicePending = false;
   mTimer = 0;
   mCookieJar->loadConfig( kapp->config());

   QString filename = locateLocal("appdata", "cookies");

   // Stay backwards compatible!
   QString filenameOld = locate("data", "kfm/cookies");
   if (!filenameOld.isEmpty())
   {
      mCookieJar->loadCookies( filenameOld );
      if (mCookieJar->saveCookies( filename))
      {
         unlink(filenameOld.ascii()); // Remove old kfm cookie file
      }
   }
   else
   {
      mCookieJar->loadCookies( filename);
   }
}

KCookieServer::~KCookieServer()
{
   if (mCookieJar->changed())
      slotSave();
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
}

int
KCookieServer::newInstance()
{
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->isSet("remove-all"))
   {
        mCookieJar->eatAllCookies();
        slotSave();
   }

   if (args->isSet("shutdown"))
   {
	quit();
   }

   return 0;
}

bool
KCookieServer::process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData)
{
    if (fun == "findCookies(QString)")
    {
        QDataStream stream(data, IO_ReadOnly);
        QString arg1;
        stream >> arg1;
        kdDebug(7104) << "got findCookies( " << arg1 << " )" << endl;
        if (cookiesPending(arg1))
        {
           kdDebug(7104) << "Blocked on pending cookies." << endl;
           CookieRequest *request = new CookieRequest;
           request->transaction = dcopClient()->beginTransaction();
           request->url = arg1;
           mRequestList->append( request );
           return true; // Talk to you later :-)
        }
        QString res = mCookieJar->findCookies(arg1);
        QDataStream stream2(replyData, IO_WriteOnly);
        stream2 << res;
        replyType = "QString";
        kdDebug(7104) << "result = " << res << endl;
        return true;
    }
    else if (fun == "addCookies(QString,QCString)")
    {
        QDataStream stream(data, IO_ReadOnly);
        QString arg1;
        QCString arg2;
        stream >> arg1 >> arg2;
        kdDebug(7104) << "got addCookies(" << arg1 << ", " << arg2.data() << ")" << endl;
        addCookies(arg1, arg2);
        replyType = "void";
        return true;
    }
    else if (fun == "reloadPolicy" )
    {
	kdDebug(7104) << "got \"reload the cookie config policy file\"" << endl;
	mCookieJar->loadConfig( kapp->config() );
        replyType = "void";
        return true;
    }
    else if (KUniqueApplication::process(fun, data, replyType, replyData))
    {
        return true;
    }
    kdDebug(7104) << "Ignoring unknown DCOP function \"" << fun.data() << "\"" << endl;
    return false;
}

bool
KCookieServer::cookiesPending( const QString &url )
{
  QString fqdn;
  QString domain;
  QString path;
  // Check whether 'url' has cookies on the pending list
  if (!KCookieJar::extractDomain( url, fqdn, domain, path))
     return false;
  for( KHttpCookie *cookie = mPendingCookies->first();
       cookie;
       cookie = mPendingCookies->next())
  {
       if (cookie->match( domain, fqdn, path))
          return true;
  }
  return false;
}

void
KCookieServer::addCookies(const QString &url, const QCString &cookieHeader)
{
    KHttpCookiePtr cookie = mCookieJar->makeCookies(url, cookieHeader);

    if (mAdvicePending)
    {
       checkCookies(cookie, true);
    }
    else
    {
       mAdvicePending = true;
       do {
          checkCookies(cookie, false);
          cookie = mPendingCookies->count() ? mPendingCookies->take(0) : 0;
       }
       while (cookie);
       mAdvicePending = false;
    }

}

void KCookieServer::checkCookies(KHttpCookie *cookie, bool queue)
{
    KCookieAdvice userAdvice = KCookieDunno;
    QString host;
    if (cookie) host = cookie->host();
    while (cookie)
    {
        KHttpCookiePtr next_cookie = cookie->next();
        KCookieAdvice advice = mCookieJar->cookieAdvice(cookie);
        if ((advice == KCookieAsk) || (advice == KCookieDunno))
        {
            // We only ask the user once, even if we get multiple
            // cookies from the same site.
            if (userAdvice == KCookieDunno)
            {
		if (queue)
                {
                    mPendingCookies->append(cookie);
                    return;
                }
                else
                {
                    kdDebug(7104) << "Asking user for advice for cookie from " << cookie->host() << endl;
                    mPendingCookies->prepend(cookie);
                    KCookieWin *kw = new KCookieWin( 0L, cookie, mCookieJar);
	            userAdvice = (KCookieAdvice) kw->advice(mCookieJar);
	            delete kw;
                    mPendingCookies->take(0);
	            // Save the cookie config if it has changed
	            mCookieJar->saveConfig( kapp->config() );
                }
	    }
	    advice = userAdvice;
        }
        switch(advice)
        {
        case KCookieAccept:
            kdDebug(7104) << "Accepting cookie from " << cookie->host() << endl;
            mCookieJar->addCookie(cookie);
	    break;
	
	case KCookieReject:
        default:
            kdDebug(7104) << "Rejecting cookie from " << cookie->host() << endl;
            delete cookie;
	    break;
        }
        cookie = next_cookie;
        if (!cookie && !queue)
        {
	   // Check if there are cookies on the pending list from the
	   // same host.
	   for( cookie = mPendingCookies->first();
                cookie;
                cookie = mPendingCookies->next())
           {
               if (cookie->host() == host)
                  break;
           }
           if (cookie)
           {
               // Found a matching cookie, remove it from the pending list.
               cookie = mPendingCookies->take();
           }
        }
    }

    // Check if we can handle any request
    for( CookieRequest *request = mRequestList->first(); request;)
    {
        if (!cookiesPending( request->url ))
        {
           QCString replyType;
           QByteArray replyData;
           QString res = mCookieJar->findCookies( request->url );
           QDataStream stream2(replyData, IO_WriteOnly);
           stream2 << res;
           replyType = "QString";
           dcopClient()->endTransaction( request->transaction,
                                       replyType, replyData);
           CookieRequest *tmp = request;
           request = mRequestList->next();
           mRequestList->removeRef( tmp );
           delete tmp;
        }
        else
        {
          request = mRequestList->next();
        }
    }
    if (mCookieJar->changed() && !mTimer)
    {
       mTimer = new QTimer();
       connect( mTimer, SIGNAL( timeout()), SLOT( slotSave()));
       mTimer->start( 1000*60*SAVE_DELAY );
    }
}

void KCookieServer::slotSave()
{
   kdDebug(7104) << "3 minutes expired. Saving cookies..." << endl;
   delete mTimer;
   mTimer = 0;
   QString filename = locateLocal("appdata", "cookies");
   mCookieJar->saveCookies(filename);
   // mCookieJar->saveConfig( kapp->config() );
}

#include "kcookieserver.moc"

