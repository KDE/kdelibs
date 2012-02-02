/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 Waldo Bastian <bastian@kde.org>

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

#include "kauthorized.h"

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QList>


#include <QCoreApplication>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <stdlib.h> // srand(), rand()
#include <unistd.h>
#include <netdb.h>
#include <kurl.h>
#include <kconfiggroup.h>

#include <QMutex>
#include <QMutexLocker>

extern bool kde_kiosk_exception;


class URLActionRule
{
  public:
#define checkExactMatch(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[s.length()-1] == QLatin1Char('!')) \
        { b = false; s.truncate(s.length()-1); } \
        else b = true;
#define checkStartWildCard(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[0] == QLatin1Char('*')) \
        { b = true; s = s.mid(1); } \
        else b = false;
#define checkEqual(s, b) \
        b = (s == QString::fromLatin1("="));

     URLActionRule(const QByteArray &act,
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

     bool baseMatch(const KUrl &url, const QString &protClass) const
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

     bool destMatch(const KUrl &url, const QString &protClass, const KUrl &base, const QString &baseClass) const
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

     QByteArray action;
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
    Q_ASSERT_X(QCoreApplication::instance(),"KAuthorizedPrivate()","There has to be an existing QCoreApplication::instance() pointer");

    KSharedConfig::Ptr config = KGlobal::config();

    Q_ASSERT_X(config,"KAuthorizedPrivate()","There has to be an existing KGlobal::config() pointer");
    if (!config) {
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

   KConfigGroup cg(KGlobal::config(), "KDE Action Restrictions");
   return cg.readEntry(genericAction, true);
}

bool KAuthorized::authorizeKAction(const QString& action)
{
   MY_D
   if (d->blockEverything) return false;
   if (!d->actionRestrictions || action.isEmpty())
      return true;

   return authorize(QLatin1String("action/") + action);
}

bool KAuthorized::authorizeControlModule(const QString &menuId)
{
   if (menuId.isEmpty() || kde_kiosk_exception)
      return true;
   KConfigGroup cg(KGlobal::config(), "KDE Control Module Restrictions");
   return cg.readEntry(menuId, true);
}

QStringList KAuthorized::authorizeControlModules(const QStringList &menuIds)
{
   KConfigGroup cg(KGlobal::config(), "KDE Control Module Restrictions");
   QStringList result;
   for(QStringList::ConstIterator it = menuIds.begin();
       it != menuIds.end(); ++it)
   {
      if (cg.readEntry(*it, true))
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
	URLActionRule("link", Any, Any, Any, QLatin1String(":internet"), Any, Any, true));
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, QLatin1String(":internet"), Any, Any, true));

  // We allow redirections to file: but not from internet protocols, redirecting to file:
  // is very popular among io-slaves and we don't want to break them
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, QLatin1String("file"), Any, Any, true));
  d->urlActionRestrictions.append(
	URLActionRule("redirect", QLatin1String(":internet"), Any, Any, QLatin1String("file"), Any, Any, false));

  // local protocols may redirect everywhere
  d->urlActionRestrictions.append(
	URLActionRule("redirect", QLatin1String(":local"), Any, Any, Any, Any, Any, true));

  // Anyone may redirect to about:
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, QLatin1String("about"), Any, Any, true));

  // Anyone may redirect to itself, cq. within it's own group
  d->urlActionRestrictions.append(
	URLActionRule("redirect", Any, Any, Any, QLatin1String("="), Any, Any, true));

  d->urlActionRestrictions.append(
	URLActionRule("redirect", QLatin1String("about"), Any, Any, Any, Any, Any, true));


  KConfigGroup cg(KGlobal::config(), "KDE URL Restrictions");
  int count = cg.readEntry("rule_count", 0);
  QString keyFormat = QString::fromLatin1("rule_%1");
  for(int i = 1; i <= count; i++)
  {
    QString key = keyFormat.arg(i);
    const QStringList rule = cg.readEntry(key, QStringList());
    if (rule.count() != 8)
      continue;
    const QByteArray action = rule[0].toLatin1();
    QString refProt = rule[1];
    QString refHost = rule[2];
    QString refPath = rule[3];
    QString urlProt = rule[4];
    QString urlHost = rule[5];
    QString urlPath = rule[6];
    bool bEnabled   = (rule[7].toLower() == QLatin1String("true"));

    if (refPath.startsWith(QLatin1String("$HOME")))
       refPath.replace(0, 5, QDir::homePath());
    else if (refPath.startsWith(QLatin1Char('~')))
       refPath.replace(0, 1, QDir::homePath());
    if (urlPath.startsWith(QLatin1String("$HOME")))
       urlPath.replace(0, 5, QDir::homePath());
    else if (urlPath.startsWith(QLatin1Char('~')))
       urlPath.replace(0, 1, QDir::homePath());

    if (refPath.startsWith(QLatin1String("$TMP")))
       refPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));
    if (urlPath.startsWith(QLatin1String("$TMP")))
       urlPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));

    d->urlActionRestrictions.append(
	URLActionRule( action, refProt, refHost, refPath, urlProt, urlHost, urlPath, bEnabled));
  }
}

void KAuthorized::allowUrlAction(const QString &action, const KUrl &_baseURL, const KUrl &_destURL)
{
  MY_D
  QMutexLocker locker((&d->mutex));
  if (authorizeUrlAction(action, _baseURL, _destURL))
     return;

  d->urlActionRestrictions.append( URLActionRule
      ( action.toLatin1(), _baseURL.protocol(), _baseURL.host(), _baseURL.path(KUrl::RemoveTrailingSlash),
        _destURL.protocol(), _destURL.host(), _destURL.path(KUrl::RemoveTrailingSlash), true));
}

bool KAuthorized::authorizeUrlAction(const QString &action, const KUrl &_baseURL, const KUrl &_destURL)
{
  MY_D
  QMutexLocker locker(&(d->mutex));
  if (d->blockEverything) return false;

  if (_destURL.isEmpty())
     return true;

  bool result = false;
  if (d->urlActionRestrictions.isEmpty())
     initUrlActionRestrictions();

  KUrl baseURL(_baseURL);
  baseURL.setPath(QDir::cleanPath(baseURL.path()));
  QString baseClass = KProtocolInfo::protocolClass(baseURL.protocol());
  KUrl destURL(_destURL);
  destURL.setPath(QDir::cleanPath(destURL.path()));
  QString destClass = KProtocolInfo::protocolClass(destURL.protocol());

  foreach(const URLActionRule &rule, d->urlActionRestrictions) {
     if ((result != rule.permission) && // No need to check if it doesn't make a difference
         (action == QLatin1String(rule.action)) &&
         rule.baseMatch(baseURL, baseClass) &&
         rule.destMatch(destURL, destClass, baseURL, baseClass))
     {
        result = rule.permission;
     }
  }
  return result;
}
