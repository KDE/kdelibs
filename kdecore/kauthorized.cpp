/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

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

#include "config.h"
#include "kauthorized.h"

#include <qdir.h>
#include <qregexp.h>
#include <QList>

#include <QString>
#include <QApplication>
#include <kglobal.h>
#include <kconfig.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <unistd.h>
#include <netdb.h>
#include <kurl.h>

#ifdef HAVE_PATHS_H
//#include <paths.h>
#endif

#include <QMutex>
#include <QMutexLocker>
#include <qglobal.h>
#ifdef __GNUC__
#warning used non public api for now
#endif

extern bool kde_kiosk_exception;


class URLActionRule
{
  public:
#define checkExactMatch(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[s.length()-1] == '!') \
        { b = false; s.truncate(s.length()-1); } \
        else b = true;
#define checkStartWildCard(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[0] == '*') \
        { b = true; s = s.mid(1); } \
        else b = false;
#define checkEqual(s, b) \
        b = (s == "=");

     URLActionRule(const QString &act,
                   const QString &bProt, const QString &bHost, const QString &bPath,
                   const QString &dProt, const QString &dHost, const QString &dPath,
                   bool perm)
                   : action(act),
                     baseProt(bProt), baseHost(bHost), basePath(bPath),
                     destProt(dProt), destHost(dHost), destPath(dPath),
                     permission(perm)
                   {
                      checkExactMatch(baseProt, baseProtWildCard);
                      checkStartWildCard(baseHost, baseHostWildCard);
                      checkExactMatch(basePath, basePathWildCard);
                      checkExactMatch(destProt, destProtWildCard);
                      checkStartWildCard(destHost, destHostWildCard);
                      checkExactMatch(destPath, destPathWildCard);
                      checkEqual(destProt, destProtEqual);
                      checkEqual(destHost, destHostEqual);
                   }

     bool baseMatch(const KURL &url, const QString &protClass)
     {
        if (baseProtWildCard)
        {
           if ( !baseProt.isEmpty() && !url.protocol().startsWith(baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        if (baseHostWildCard)
        {
           if (!baseHost.isEmpty() && !url.host().endsWith(baseHost))
              return false;
        }
        else
        {
           if (url.host() != baseHost)
              return false;
        }
        if (basePathWildCard)
        {
           if (!basePath.isEmpty() && !url.path().startsWith(basePath))
              return false;
        }
        else
        {
           if (url.path() != basePath)
              return false;
        }
        return true;
     }

     bool destMatch(const KURL &url, const QString &protClass, const KURL &base, const QString &baseClass)
     {
        if (destProtEqual)
        {
           if ( (url.protocol() != base.protocol()) &&
                (protClass.isEmpty() || baseClass.isEmpty() || protClass != baseClass) )
              return false;
        }
        else if (destProtWildCard)
        {
           if ( !destProt.isEmpty() && !url.protocol().startsWith(destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        if (destHostWildCard)
        {
           if (!destHost.isEmpty() && !url.host().endsWith(destHost))
              return false;
        }
        else if (destHostEqual)
        {
           if (url.host() != base.host())
              return false;
        }
        else
        {
           if (url.host() != destHost)
              return false;
        }
        if (destPathWildCard)
        {
           if (!destPath.isEmpty() && !url.path().startsWith(destPath))
              return false;
        }
        else
        {
           if (url.path() != destPath)
              return false;
        }
        return true;
     }

     QString action;
     QString baseProt;
     QString baseHost;
     QString basePath;
     QString destProt;
     QString destHost;
     QString destPath;
     bool baseProtWildCard : 1;
     bool baseHostWildCard : 1;
     bool basePathWildCard : 1;
     bool destProtWildCard : 1;
     bool destHostWildCard : 1;
     bool destPathWildCard : 1;
     bool destProtEqual    : 1;
     bool destHostEqual    : 1;
     bool permission;
};

class KAuthorizedPrivate {
public:
  KAuthorizedPrivate()
    :   actionRestrictions( false ), blockEverything(false),mutex(QMutex::Recursive)
  {
    Q_ASSERT_X(qApp,"KAuthorizedPrivate()","There has to be an existing qapp pointer");
    Q_ASSERT_X(!qApp->applicationName().isEmpty(),"KAuthorizedPrivate()","There has to be an application name set (See QApplication::setApplicationName)");
    
    KConfig* config = KGlobal::config();

    Q_ASSERT_X(config,"KAuthorizedPrivate()","There has to be an existing KGlobal::config() pointer");
    if (config==0) {
      blockEverything=true;
      return;
    }
    actionRestrictions = config->hasGroup("KDE Action Restrictions" ) && !kde_kiosk_exception;
  }

  ~KAuthorizedPrivate()
  {
  }

  bool actionRestrictions : 1;
  bool blockEverything : 1;
  QList<URLActionRule> urlActionRestrictions;
  QMutex mutex;
};

Q_GLOBAL_STATIC(KAuthorizedPrivate,authPrivate)
#define MY_D KAuthorizedPrivate *d=authPrivate();


bool KAuthorized::authorize(const QString &genericAction)
{
   MY_D
   if (d->blockEverything) return false;

   if (!d->actionRestrictions)
      return true;

   KConfigGroup cg( KGlobal::config(), "KDE Action Restrictions" );
   return cg.readBoolEntry(genericAction, true);
}

bool KAuthorized::authorizeKAction(const char *action)
{
   MY_D
   if (d->blockEverything) return false;
   if (!d->actionRestrictions || !action)
      return true;

   static const QString &action_prefix = KGlobal::staticQString( "action/" );

   return authorize(action_prefix + action);
}

bool KAuthorized::authorizeControlModule(const QString &menuId)
{
   if (menuId.isEmpty() || kde_kiosk_exception)
      return true;
   KConfigGroup cg( KGlobal::config(), "KDE Control Module Restrictions" );
   return cg.readBoolEntry(menuId, true);
}

QStringList KAuthorized::authorizeControlModules(const QStringList &menuIds)
{
   KConfigGroup cg( KGlobal::config(), "KDE Control Module Restrictions" );
   QStringList result;
   for(QStringList::ConstIterator it = menuIds.begin();
       it != menuIds.end(); ++it)
   {
      if (cg.readBoolEntry(*it, true))
         result.append(*it);
   }
   return result;
}

static void initUrlActionRestrictions()
{
  MY_D
  const QString Any;

  d->urlActionRestrictions.clear();
  d->urlActionRestrictions.append(
	URLActionRule("open", Any, Any, Any, Any, Any, Any, true));
  d->urlActionRestrictions.append(
	URLActionRule("list", Any, Any, Any, Any, Any, Any, true));
// TEST:
//  d->urlActionRestrictions.append(
//	URLActionRule("list", Any, Any, Any, Any, Any, Any, false));
//  d->urlActionRestrictions.append(
//	URLActionRule("list", Any, Any, Any, "file", Any, QDir::homePath(), true));
  d->urlActionRestrictions.append(
	URLActionRule("link", Any, Any, Any, ":internet", Any, Any, true));
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, ":internet", Any, Any, true));

  // We allow redirections to file: but not from internet protocols, redirecting to file:
  // is very popular among io-slaves and we don't want to break them
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, "file", Any, Any, true));
  d->urlActionRestrictions.append(
	URLActionRule("redirect", ":internet", Any, Any, "file", Any, Any, false));

  // local protocols may redirect everywhere
  d->urlActionRestrictions.append(
	URLActionRule("redirect", ":local", Any, Any, Any, Any, Any, true));

  // Anyone may redirect to about:
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, "about", Any, Any, true));

  // Anyone may redirect to itself, cq. within it's own group
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, "=", Any, Any, true));

  KConfigGroup cg( KGlobal::config(), "KDE URL Restrictions" );
  int count = cg.readNumEntry("rule_count");
  QString keyFormat = QString("rule_%1");
  for(int i = 1; i <= count; i++)
  {
    QString key = keyFormat.arg(i);
    QStringList rule = cg.readListEntry(key);
    if (rule.count() != 8)
      continue;
    QString action = rule[0];
    QString refProt = rule[1];
    QString refHost = rule[2];
    QString refPath = rule[3];
    QString urlProt = rule[4];
    QString urlHost = rule[5];
    QString urlPath = rule[6];
    QString strEnabled = rule[7].toLower();

    bool bEnabled = (strEnabled == "true");

    if (refPath.startsWith("$HOME"))
       refPath.replace(0, 5, QDir::homePath());
    else if (refPath.startsWith("~"))
       refPath.replace(0, 1, QDir::homePath());
    if (urlPath.startsWith("$HOME"))
       urlPath.replace(0, 5, QDir::homePath());
    else if (urlPath.startsWith("~"))
       urlPath.replace(0, 1, QDir::homePath());

    if (refPath.startsWith("$TMP"))
       refPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));
    if (urlPath.startsWith("$TMP"))
       urlPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));

    d->urlActionRestrictions.append(
	URLActionRule( action, refProt, refHost, refPath, urlProt, urlHost, urlPath, bEnabled));
  }
}

void KAuthorized::allowURLAction(const QString &action, const KURL &_baseURL, const KURL &_destURL)
{
  MY_D
  QMutexLocker locker((&d->mutex));
  if (authorizeURLAction(action, _baseURL, _destURL))
     return;

  d->urlActionRestrictions.append( URLActionRule
        ( action, _baseURL.protocol(), _baseURL.host(), _baseURL.path(-1),
                  _destURL.protocol(), _destURL.host(), _destURL.path(-1), true));
}

bool KAuthorized::authorizeURLAction(const QString &action, const KURL &_baseURL, const KURL &_destURL)
{
  MY_D
  QMutexLocker locker(&(d->mutex));
  if (d->blockEverything) return false;

  if (_destURL.isEmpty())
     return true;

  bool result = false;
  if (d->urlActionRestrictions.isEmpty())
     initUrlActionRestrictions();

  KURL baseURL(_baseURL);
  baseURL.setPath(QDir::cleanPath(baseURL.path()));
  QString baseClass = KProtocolInfo::protocolClass(baseURL.protocol());
  KURL destURL(_destURL);
  destURL.setPath(QDir::cleanPath(destURL.path()));
  QString destClass = KProtocolInfo::protocolClass(destURL.protocol());

  foreach(URLActionRule rule, d->urlActionRestrictions) {
     if ((result != rule.permission) && // No need to check if it doesn't make a difference
         (action == rule.action) &&
         rule.baseMatch(baseURL, baseClass) &&
         rule.destMatch(destURL, destClass, baseURL, baseClass))
     {
        result = rule.permission;
     }
  }
  return result;
}
