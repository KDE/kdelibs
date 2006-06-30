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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------------
//
// KDE Cookie Server

#ifndef KCOOKIESERVER_H
#define KCOOKIESERVER_H

#include <qstringlist.h>
#include <kdedmodule.h>
#include <dbus/qdbus.h>

class KHttpCookieList;
class KCookieJar;
class KHttpCookie;
class QTimer;
class RequestList;
class KConfig;

class KCookieServer : public KDEDModule
{
  Q_OBJECT
public:
  KCookieServer();
  ~KCookieServer();

public Q_SLOTS:
  QString findCookies(QString, qlonglong, const QDBusMessage &msg);
  QStringList findDomains();
  QStringList findCookies(QList<int>,QString,QString,QString,QString);
  QString findDOMCookies(QString);
  QString findDOMCookies(QString, qlonglong);
  void addCookies(QString, QByteArray, qlonglong);
  void deleteCookie(QString, QString, QString, QString);
  void deleteCookiesFromDomain(QString);
  void deleteSessionCookies(qlonglong);
  void deleteSessionCookiesFor(QString, qlonglong);
  void deleteAllCookies();
  void addDOMCookies(QString, QByteArray, qlonglong);
  /**
   * Sets the cookie policy for the domain associated with the specified URL.
   */
  void setDomainAdvice(QString url, QString advice);
  /**
   * Returns the cookie policy in effect for the specified URL.
   */
  QString getDomainAdvice(QString url);
  void reloadPolicy();
  void shutdown();

public:
  bool cookiesPending(const QString &url, KHttpCookieList *cookieList=0);
  void addCookies(const QString &url, const QByteArray &cookieHeader,
                  qlonglong windowId, bool useDOMFormat);
  void checkCookies(KHttpCookieList *cookieList);

public Q_SLOTS:
  void slotSave();
  void slotDeleteSessionCookies(qlonglong);

protected:
  KCookieJar *mCookieJar;
  KHttpCookieList *mPendingCookies;
  RequestList *mRequestList;
  QTimer *mTimer;
  bool mAdvicePending;
  KConfig *mConfig;

private:
  virtual int newInstance(QList<QByteArray>) { return 0; }
  bool cookieMatches(KHttpCookie*, QString, QString, QString, QString);
  void putCookie(QStringList&, KHttpCookie*, const QList<int>&);
  void saveCookieJar();
};

#endif
