/*
    This file is part of the KDE File Manager

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
#include <kstddirs.h>
#include <qtimer.h>
#include <unistd.h>

KCookieServer::KCookieServer(int argc, char *argv[]) 
  : KUniqueApplication( argc, argv, "kcookiejar" )
{
   mCookieJar = new KCookieJar;
   mPendingCookies = new KCookieList;
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
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
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
        kdebug(KDEBUG_INFO, 7104, "got findCookies( %s )", arg1.ascii());
        QString res = mCookieJar->findCookies(arg1); 
        QDataStream stream2(replyData, IO_WriteOnly);
        stream2 << res;
        replyType = "QString";
        kdebug(KDEBUG_INFO, 7104, "result = %s", res.ascii());
        return true;
    }
    else if (fun == "addCookies(QString,QCString)")
    {
        QDataStream stream(data, IO_ReadOnly);
        QString arg1;
        QCString arg2;
        stream >> arg1 >> arg2;
        kdebug(KDEBUG_INFO, 7104, "got addCookies(%s, %s)", arg1.ascii(), arg2.data());
        addCookies(arg1, arg2); 
        replyType = "void";
        return true;
    }
    else if (KUniqueApplication::process(fun, data, replyType, replyData))
    {
        return true;
    }
    kdebug(KDEBUG_INFO, 7104, "Ignoring unknown DCOP function \"%s\"", fun.data());
    return false;
}

void
KCookieServer::addCookies(QString url, QCString cookieHeader)
{
    KCookiePtr cookie = mCookieJar->makeCookies(url, cookieHeader);

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

void KCookieServer::checkCookies(KCookie *cookie, bool queue)
{
    KCookieAdvice userAdvice = KCookieDunno;
    QString host;
    if (cookie) host = cookie->host();
    while (cookie)
    {
        KCookiePtr next_cookie = cookie->next();
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
                    kdebug(KDEBUG_INFO, 7104, "Asking user for advice for cookie from %s", cookie->host().ascii());
                    KCookieWin *kw = new KCookieWin( 0L, cookie);
	            userAdvice = (KCookieAdvice) kw->advice(mCookieJar);
	            delete kw;
	            // Save the cookie config if it has changed
	            mCookieJar->saveConfig( kapp->config() ); 
                }
	    }
	    advice = userAdvice;
        }
        switch(advice)
        {
        case KCookieAccept:
            kdebug(KDEBUG_INFO, 7104, "Accepting cookie from %s", cookie->host().ascii());
            mCookieJar->addCookie(cookie);
	    break;
	
	case KCookieReject:
        default:
            kdebug(KDEBUG_INFO, 7104, "Rejecting cookie from %s", cookie->host().ascii());
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
    if (mCookieJar->changed() && !mTimer)
    {
       mTimer = new QTimer();
       connect( mTimer, SIGNAL( timeout()), SLOT( slotSave()));
       mTimer->start( 1000*60*SAVE_DELAY );
    }
}

void KCookieServer::slotSave()
{
   kdebug(KDEBUG_INFO, 7104, "Saving cookie stuff!\n");
   
   delete mTimer;
   mTimer = 0;
   QString filename = locateLocal("appdata", "cookies");
   mCookieJar->saveCookies(filename);

   mCookieJar->saveConfig( kapp->config());
}

#include "kcookieserver.moc"

