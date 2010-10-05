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

#include "kcookieserver.h"

#define SAVE_DELAY 3 // Save after 3 minutes

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtDBus/QtDBus>

#include <kconfig.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "kcookiejar.h"
#include "kcookiewin.h"
#include "kcookieserveradaptor.h"
#include <kpluginfactory.h>
#include <kpluginloader.h>


#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)

K_PLUGIN_FACTORY(KdedCookieServerFactory,
                 registerPlugin<KCookieServer>();
    )
K_EXPORT_PLUGIN(KdedCookieServerFactory("kcookiejar"))

// Cookie field indexes
enum CookieDetails { CF_DOMAIN=0, CF_PATH, CF_NAME, CF_HOST,
                     CF_VALUE, CF_EXPIRE, CF_PROVER, CF_SECURE };


class CookieRequest {
public:
   QDBusMessage reply;
   QString url;
   bool DOM;
   qlonglong windowId;
};

template class  QList<CookieRequest*>;

class RequestList : public QList<CookieRequest*>
{
public:
   RequestList() : QList<CookieRequest*>() { }
};

KCookieServer::KCookieServer(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
   (void)new KCookieServerAdaptor(this);
   mCookieJar = new KCookieJar;
   mPendingCookies = new KHttpCookieList;
   mRequestList = new RequestList;
   mAdvicePending = false;
   mTimer = new QTimer();
   mTimer->setSingleShot(true);
   connect(mTimer, SIGNAL( timeout()), SLOT( slotSave()));
   mConfig = new KConfig("kcookiejarrc");
   mCookieJar->loadConfig( mConfig );

   const QString filename = KStandardDirs::locateLocal("data", "kcookiejar/cookies");

   // Stay backwards compatible!
   const QString filenameOld = KStandardDirs::locate("data", "kfm/cookies");
   if (!filenameOld.isEmpty())
   {
      mCookieJar->loadCookies( filenameOld );
      if (mCookieJar->saveCookies( filename))
      {
         QFile::remove(filenameOld); // Remove old kfm cookie file
      }
   }
   else
   {
      mCookieJar->loadCookies( filename);
   }
   connect(this, SIGNAL(windowUnregistered(qlonglong)),
           this, SLOT(slotDeleteSessionCookies(qlonglong)));
}

KCookieServer::~KCookieServer()
{
   slotSave();
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
   delete mConfig;
}

bool KCookieServer::cookiesPending( const QString &url, KHttpCookieList *cookieList )
{
    QString fqdn;
    QString path;
    // Check whether 'url' has cookies on the pending list
    if (mPendingCookies->isEmpty())
        return false;
    if (!KCookieJar::parseUrl(url, fqdn, path))
        return false;

    QStringList domains;
    mCookieJar->extractDomains(fqdn, domains);
    Q_FOREACH(const KHttpCookie& cookie, *mPendingCookies) {
        if (cookie.match( fqdn, domains, path)) {
            if (!cookieList)
                return true;
            cookieList->append(cookie);
        }
    }
    if (!cookieList)
        return false;
    return cookieList->isEmpty();
}

void KCookieServer::addCookies( const QString &url, const QByteArray &cookieHeader,
                               qlonglong windowId, bool useDOMFormat )
{
    KHttpCookieList cookieList;
    if (useDOMFormat)
       cookieList = mCookieJar->makeDOMCookies(url, cookieHeader, windowId);
    else
       cookieList = mCookieJar->makeCookies(url, cookieHeader, windowId);

    checkCookies(&cookieList);

    *mPendingCookies += cookieList;

    if (!mAdvicePending)
    {
       mAdvicePending = true;
       while (!mPendingCookies->isEmpty())
       {
          checkCookies(0);
       }
       mAdvicePending = false;
    }
}

void KCookieServer::checkCookies( KHttpCookieList *cookieList)
{
    KHttpCookieList *list;

    if (cookieList)
       list = cookieList;
    else
       list = mPendingCookies;

    QMutableListIterator<KHttpCookie> cookieIterator(*list);
    while (cookieIterator.hasNext()) {
        KHttpCookie& cookie = cookieIterator.next();
        const KCookieAdvice advice = mCookieJar->cookieAdvice(cookie);
        switch(advice) {
        case KCookieAccept:
            mCookieJar->addCookie(cookie);
            cookieIterator.remove();
            break;

        case KCookieReject:
            cookieIterator.remove();
            break;

        default:
            break;
        }
    }

    if (cookieList || list->isEmpty())
       return;

    // Collect all pending cookies with the same host as the first pending cookie
    const KHttpCookie& currentCookie = mPendingCookies->first();
    KHttpCookieList currentList;
    currentList.append(currentCookie);
    const QString currentHost = currentCookie.host();
    QList<int> shownCookies; shownCookies << 0;
    for (int i = 1 /*first already done*/; i < mPendingCookies->count(); ++i) {
        const KHttpCookie& cookie = (*mPendingCookies)[i];
        if (cookie.host() == currentHost) {
            currentList.append(cookie);
            shownCookies << i;
        }
    }
    //kDebug() << shownCookies;

    KCookieWin *kw = new KCookieWin( 0L, currentList,
                                     mCookieJar->preferredDefaultPolicy(),
                                     mCookieJar->showCookieDetails() );
    KCookieAdvice userAdvice = kw->advice(mCookieJar, currentCookie);
    delete kw;
    // Save the cookie config if it has changed
    mCookieJar->saveConfig( mConfig );

    // Apply the user's choice to all cookies that are currently
    // queued for this host (or just the first one, if the user asks for that).
    QMutableListIterator<KHttpCookie> cookieIterator2(*mPendingCookies);
    int pendingCookieIndex = -1;
    while (cookieIterator2.hasNext()) {
        ++pendingCookieIndex;
        KHttpCookie& cookie = cookieIterator2.next();
        if (cookie.host() != currentHost)
            continue;
        if (mCookieJar->preferredDefaultPolicy() == KCookieJar::ApplyToShownCookiesOnly
            && !shownCookies.contains(pendingCookieIndex)) {
            // User chose "only those cookies", and this one was added while the dialog was up -> skip
            break;
        }
        switch(userAdvice) {
           case KCookieAccept:
               mCookieJar->addCookie(cookie);
               cookieIterator2.remove();
               break;

           case KCookieReject:
               cookieIterator2.remove();
               break;

           default:
               kWarning() << "userAdvice not accept or reject, this should never happen!";
               break;
        }
    }

    // Check if we can handle any request
    QMutableListIterator<CookieRequest *> requestIterator(*mRequestList);
    while (requestIterator.hasNext()) {
        CookieRequest *request = requestIterator.next();
        if (!cookiesPending(request->url)) {
           const QString res = mCookieJar->findCookies(request->url, request->DOM, request->windowId);

           QDBusConnection::sessionBus().send(request->reply.createReply(res));
           delete request;
           requestIterator.remove();
        }
    }

    saveCookieJar();
}

void KCookieServer::slotSave()
{
   if (mCookieJar->changed())
   {
      QString filename = KStandardDirs::locateLocal("data", "kcookiejar/cookies");
      mCookieJar->saveCookies(filename);
   }
}

void KCookieServer::saveCookieJar()
{
    if( mTimer->isActive() )
        return;

    mTimer->start( 1000*60*SAVE_DELAY );
}

void KCookieServer::putCookie( QStringList& out, const KHttpCookie& cookie,
                               const QList<int>& fields )
{
    foreach ( int i, fields ) {
       switch(i)
        {
         case CF_DOMAIN :
            out << cookie.domain();
            break;
         case CF_NAME :
            out << cookie.name();
            break;
         case CF_PATH :
            out << cookie.path();
            break;
         case CF_HOST :
            out << cookie.host();
            break;
         case CF_VALUE :
            out << cookie.value();
            break;
         case CF_EXPIRE :
            out << QString::number(cookie.expireDate());
            break;
         case CF_PROVER :
            out << QString::number(cookie.protocolVersion());
            break;
         case CF_SECURE :
            out << QString::number(cookie.isSecure() ? 1 : 0);
            break;
         default :
            out << QString();
        }
    }
}

bool KCookieServer::cookieMatches(const KHttpCookie& c,
                                  const QString &domain, const QString &fqdn,
                                  const QString &path, const QString &name)
{
    const bool hasDomain = !domain.isEmpty();
    return (((hasDomain && c.domain() == domain) || fqdn == c.host()) &&
            (c.path() == path) && (c.name() == name) &&
            (!c.isExpired()));
}


// DBUS function
QString KCookieServer::listCookies(const QString &url)
{
    return findCookies(url, 0);
}

// DBUS function
QString KCookieServer::findCookies(const QString &url, qlonglong windowId)
{
   if (cookiesPending(url))
   {
      CookieRequest *request = new CookieRequest;
      message().setDelayedReply(true);
      request->reply = message();
      request->url = url;
      request->DOM = false;
      request->windowId = windowId;
      mRequestList->append( request );
      return QString(); // Talk to you later :-)
   }

   QString cookies = mCookieJar->findCookies(url, false, windowId);
   saveCookieJar();
   return cookies;
}

// DBUS function
QStringList
KCookieServer::findDomains()
{
   QStringList result;
   QStringListIterator it (mCookieJar->getDomainList());
   while (it.hasNext())
   {
       // Ignore domains that have policy set for but contain
       // no cookies whatsoever...
       const QString domain = it.next();
       const KHttpCookieList* list =  mCookieJar->getCookieList(domain, "");
       if ( list && !list->isEmpty() )
          result << domain;
   }
   return result;
}

// DBUS function
QStringList
KCookieServer::findCookies(const QList<int> &fields,
                           const QString &domain,
                           const QString &fqdn,
                           const QString &path,
                           const QString &name)
{
    QStringList result;
    const bool allDomCookies = name.isEmpty();

    const KHttpCookieList* list =  mCookieJar->getCookieList(domain, fqdn);
    if (list && !list->isEmpty()) {
        Q_FOREACH(const KHttpCookie& cookie, *list) {
            if (!allDomCookies) {
                if (cookieMatches(cookie, domain, fqdn, path, name)) {
                    putCookie(result, cookie, fields);
                    break;
                }
            } else {
                putCookie(result, cookie, fields);
            }
        }
    }
    return result;
}

// DBUS function
QString
KCookieServer::findDOMCookies(const QString &url)
{
   return findDOMCookies(url, 0);
}

// DBUS function
QString
KCookieServer::findDOMCookies(const QString &url, qlonglong windowId)
{
   // We don't wait for pending cookies because it locks up konqueror
   // which can cause a deadlock if it happens to have a popup-menu up.
   // Instead we just return pending cookies as if they had been accepted already.
   KHttpCookieList pendingCookies;
   cookiesPending(url, &pendingCookies);

   return mCookieJar->findCookies(url, true, windowId, &pendingCookies);
}

// DBUS function
void
KCookieServer::addCookies(const QString &arg1, const QByteArray &arg2, qlonglong arg3)
{
   addCookies(arg1, arg2, arg3, false);
}

// DBUS function
void
KCookieServer::deleteCookie(const QString &domain, const QString &fqdn,
                            const QString &path, const QString &name)
{
    KHttpCookieList* cookieList = mCookieJar->getCookieList( domain, fqdn );
    if (cookieList && !cookieList->isEmpty()) {
        KHttpCookieList::Iterator itEnd = cookieList->end();
        for (KHttpCookieList::Iterator it = cookieList->begin(); it != itEnd; ++it)
        {
            if (cookieMatches(*it, domain, fqdn, path, name)) {
                mCookieJar->eatCookie(it);
                saveCookieJar();
                break;
            }
        }
    }
}

// DBUS function
void
KCookieServer::deleteCookiesFromDomain(const QString &domain)
{
   mCookieJar->eatCookiesForDomain(domain);
   saveCookieJar();
}


// Qt function
void
KCookieServer::slotDeleteSessionCookies( qlonglong windowId )
{
   deleteSessionCookies(windowId);
}

// DBUS function
void
KCookieServer::deleteSessionCookies( qlonglong windowId )
{
  mCookieJar->eatSessionCookies( windowId );
  saveCookieJar();
}

void
KCookieServer::deleteSessionCookiesFor(const QString &fqdn, qlonglong windowId)
{
  mCookieJar->eatSessionCookies( fqdn, windowId );
  saveCookieJar();
}

// DBUS function
void
KCookieServer::deleteAllCookies()
{
   mCookieJar->eatAllCookies();
   saveCookieJar();
}

// DBUS function
void
KCookieServer::addDOMCookies(const QString &url, const QByteArray &cookieHeader, qlonglong windowId)
{
   addCookies(url, cookieHeader, windowId, true);
}

// DBUS function
bool
KCookieServer::setDomainAdvice(const QString &url, const QString &advice)
{
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseUrl(url, fqdn, dummy))
   {
      QStringList domains;
      mCookieJar->extractDomains(fqdn, domains);

      mCookieJar->setDomainAdvice(domains[domains.count() > 3 ? 3 : 0],
                                  KCookieJar::strToAdvice(advice));
      // Save the cookie config if it has changed
      mCookieJar->saveConfig( mConfig );
      return true;
   }
   return false;
}

// DBUS function
QString
KCookieServer::getDomainAdvice(const QString &url)
{
   KCookieAdvice advice = KCookieDunno;
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseUrl(url, fqdn, dummy))
   {
      QStringList domains;
      mCookieJar->extractDomains(fqdn, domains);

      QStringListIterator it (domains);
      while ( (advice == KCookieDunno) && it.hasNext() )
      {
         // Always check advice in both ".domain" and "domain". Note
         // that we only want to check "domain" if it matches the
         // fqdn of the requested URL.
        const QString& domain = it.next();
         if ( domain.at(0) == '.' || domain == fqdn )
            advice = mCookieJar->getDomainAdvice(domain);
      }
      if (advice == KCookieDunno)
         advice = mCookieJar->getGlobalAdvice();
   }
   return KCookieJar::adviceToStr(advice);
}

// DBUS function
void
KCookieServer::reloadPolicy()
{
   mCookieJar->loadConfig( mConfig, true );
}

// DBUS function
void
KCookieServer::shutdown()
{
   deleteLater();
}

#include "kcookieserver.moc"

