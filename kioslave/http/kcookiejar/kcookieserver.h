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
// $Id: kcookieserver.h 465272 2005-09-29 09:47:40Z mueller $

#ifndef KCOOKIESERVER_H
#define KCOOKIESERVER_H

#include <qstringlist.h>
#include <kded/kdedmodule.h>

class KHttpCookieList;
class KCookieJar;
class KHttpCookie;
class QTimer;
class RequestList;
class DCOPClient;
class KConfig;

class KCookieServer : public KDEDModule
{
  Q_OBJECT
  K_DCOP
public:
  KCookieServer(const QCString &);
  ~KCookieServer();

k_dcop:
  QString findCookies(QString);
  QString findCookies(QString, long);
  QStringList findDomains();
  QStringList findCookies(QValueList<int>,QString,QString,QString,QString);
  QString findDOMCookies(QString);
  QString findDOMCookies(QString, long);
  void addCookies(QString, QCString, long);
  void deleteCookie(QString, QString, QString, QString);
  void deleteCookiesFromDomain(QString);
  void deleteSessionCookies(long);
  void deleteSessionCookiesFor(QString, long);
  void deleteAllCookies();
  void addDOMCookies(QString, QCString, long);
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
  void addCookies(const QString &url, const QCString &cookieHeader,
                  long windowId, bool useDOMFormat);
  void checkCookies(KHttpCookieList *cookieList);

public slots:
  void slotSave();
  void slotDeleteSessionCookies(long);

protected:
  KCookieJar *mCookieJar;
  KHttpCookieList *mPendingCookies;
  RequestList *mRequestList;
  QTimer *mTimer;
  bool mAdvicePending;
  DCOPClient *mOldCookieServer;
  KConfig *mConfig;

private:
  virtual int newInstance(QValueList<QCString>) { return 0; }
  bool cookieMatches(KHttpCookie*, QString, QString, QString, QString);
  void putCookie(QStringList&, KHttpCookie*, const QValueList<int>&);
  void saveCookieJar();
};

#endif
