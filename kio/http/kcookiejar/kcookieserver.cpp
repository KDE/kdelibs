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
#include <kconfig.h>

// Cookie field indexes
enum CookieDetails { CF_DOMAIN=0, CF_PATH, CF_NAME, CF_HOST,
                     CF_VALUE, CF_EXPIRE, CF_PROVER, CF_SECURE };

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
              :KUniqueApplication()
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

int KCookieServer::newInstance()
{
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->isSet("remove-all"))
   {
        mCookieJar->eatAllCookies();
        slotSave();
   }
   if (args->isSet("remove"))
   {
        mCookieJar->eatCookiesForDomain(args->getOption("remove"));
        slotSave();
   }

   if (args->isSet("shutdown"))
   {
        quit();
   }
   if(args->isSet("reload-config"))
   {
        mCookieJar->loadConfig( kapp->config(), true );
   }
   return 0;
}

bool KCookieServer::process(const QCString &fun, const QByteArray &data,
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
    else if (fun == "findDomains()")
    {
        QStringList result;
        const QStringList *domains = mCookieJar->getDomainList();
        for (QStringList::ConstIterator domIt = domains->begin(); domIt != domains->end(); ++domIt)
        {
            // Ignore domains that have policy set for but contain
            // no cookies whatsoever...
            if ( mCookieJar->getCookieList(*domIt)->count() != 0 )
                result << *domIt;
        }

        QDataStream reply(replyData, IO_WriteOnly);
        reply << result;
        replyType = "QStringList";
        return true;
    }
    else if (fun == "findCookies(QValueList<int>,QString,QString,QString,QString)")
    {
        QString domain;             // domain name as returned by findDomain
        QString cdomain;            // cookie domain to match     |
        QString path;               // cookie path to match     <-| all empty OR full :)
        QString name;               // cookie name to match       |
        QValueList<int> fields;     // Requested fields in order

        QDataStream stream(data, IO_ReadOnly);
        stream >> fields >> domain >> cdomain >> path >> name;
        bool allDomCookies = name.isEmpty();

        QStringList result;
        KHttpCookieList *domcookies =  const_cast<KHttpCookieList*>(mCookieJar->getCookieList(domain));
        if (domcookies)
        {
            for (KHttpCookiePtr cIt = domcookies->first(); cIt; cIt = domcookies->next() )
            {
                if (!allDomCookies)
                {
                    if ( cookieMatches(cIt, cdomain, path, name) )
                    {
                        putCookie(&result, cIt, &fields);
                        break;
                    }
                }
                else
                    putCookie(&result, cIt, &fields);
            }
        }
        QDataStream reply(replyData, IO_WriteOnly);
        reply << result;
        replyType = "QStringList";
        return true;
    }
    else if (fun == "addCookies(QString,QCString,long)")
    {
        QDataStream stream(data, IO_ReadOnly);
        QString arg1;
        QCString arg2;
        long arg3;
        stream >> arg1 >> arg2 >> arg3;
        kdDebug(7104) << "got addCookies(" << arg1 << ", " << arg2.data() << ", " << arg3 << ")" << endl;
        addCookies(arg1, arg2, arg3);
        replyType = "void";
        return true;
    }
    else if (fun == "deleteCookie(QString,QString,QString,QString)")
    {
        QString domain;     // name as returned by findDomains
        QString cdomain;    // cookie domain to be matched
        QString path;       // cookie path to be matched
        QString name;       // cookie name to be matched

        QDataStream stream(data, IO_ReadOnly);
        stream >> domain >> cdomain >> path >> name;
        KHttpCookieList *domcookies =  const_cast<KHttpCookieList*>(mCookieJar->getCookieList(domain));
        if (domcookies)
        {
            for (KHttpCookiePtr cIt = domcookies->first(); cIt; cIt = domcookies->next() )
            {
                if (cookieMatches(cIt, cdomain, path, name))
                {
                    mCookieJar->eatCookie(cIt);
                    if (!mTimer)
                        saveCookieJar();
                    break;
                }
            }
        }
        replyType = "void";
        return true;
    }
    else if (fun == "deleteCookiesFromDomain(QString)")
    {
        QString domain;
        //delete all cookies originating from domain
        QDataStream stream(data, IO_ReadOnly);
        stream >> domain;
        mCookieJar->eatCookiesForDomain(domain);
        if (!mTimer)
            saveCookieJar();
        replyType = "void";
        return true;
    }
    else if (fun == "deleteAllCookies()")
    {
        mCookieJar->eatAllCookies();
        if (!mTimer)
            saveCookieJar();
        replyType = "void";
        return true;
    }
    else if (fun == "reloadPolicy" )
    {
        kdDebug(7104) << "got \"reload the cookie config policy file\"..." << endl;
        mCookieJar->loadConfig( kapp->config(), true );
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

bool KCookieServer::cookiesPending( const QString &url )
{
  QString fqdn;
  QStringList domains;
  QString path;
  // Check whether 'url' has cookies on the pending list
  if (mPendingCookies->isEmpty())
     return false;
  if (!KCookieJar::parseURL(url, fqdn, path))
     return false;
  if (!KCookieJar::extractDomains( fqdn, domains))
     return false;
  for( KHttpCookie *cookie = mPendingCookies->first();
       cookie;
       cookie = mPendingCookies->next())
  {
       if (cookie->match( domains, path))
          return true;
  }
  return false;
}

void KCookieServer::addCookies( const QString &url, const QCString &cookieHeader,
                               long windowId )
{
    KHttpCookiePtr cookie = mCookieJar->makeCookies(url, cookieHeader, windowId);

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

void KCookieServer::checkCookies( KHttpCookie *cookie, bool queue )
{
    QString host;
    KCookieAdvice userAdvice = KCookieDunno;
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
            kdDebug(7104) << "Accepting cookies from " << cookie->host() << endl;
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
        saveCookieJar();
}

void KCookieServer::slotSave()
{
   delete mTimer;
   mTimer = 0;
   kdDebug(7104) << "Saving cookies..." << endl;
   QString filename = locateLocal("appdata", "cookies");
   mCookieJar->saveCookies(filename);
}

void KCookieServer::saveCookieJar()
{
    if( mTimer )
        return;

    mTimer = new QTimer();
    connect( mTimer, SIGNAL( timeout()), SLOT( slotSave()));
    mTimer->start( 1000*60*SAVE_DELAY );
}

void KCookieServer::putCookie( QStringList *out, KHttpCookie *cookie,
                               const QValueList<int> *fields )
{
    QValueList<int>::ConstIterator i = fields->begin();
    for ( ; i != fields->end(); ++i )
    {
        switch(*i) {
         case CF_DOMAIN :
            *out << cookie->domain();
            break;
         case CF_NAME :
            *out << cookie->name();
            break;
         case CF_PATH :
            *out << cookie->path();
            break;
         case CF_HOST :
            *out << cookie->host();
            break;
         case CF_VALUE :
            *out << cookie->value();
            break;
         case CF_EXPIRE :
            *out << QString::number(cookie->expireDate());
            break;
         case CF_PROVER :
            *out << QString::number(cookie->protocolVersion());
            break;
         case CF_SECURE :
            *out << QString::number( cookie->isSecure() ? 1 : 0 );
            break;
         default :
            *out << QString::null;
        }
    }
}

bool KCookieServer::cookieMatches( KHttpCookiePtr c, QString domain,
                                   QString path, QString name )
{
    if( c )
    {
        return
       (c->domain() == domain) &&
       (c->path()   == path)   &&
       (c->name()   == name);
    }
    return false;
}

#include "kcookieserver.moc"

