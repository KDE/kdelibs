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

#ifndef KCOOKIESERVER_H
#define KCOOKIESERVER_H

#include <dcopobject.h>
#include <kuniqueapp.h>

class KHttpCookieList;
class KCookieJar;
class KHttpCookie;
class QTimer;
class RequestList;

class KCookieServer : public KUniqueApplication
{
  Q_OBJECT 
public:
  KCookieServer();
  ~KCookieServer();

  virtual int newInstance();

  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);

  bool cookiesPending(const QString &url);
  void addCookies(const QString &url, const QCString &cookieHeader);
  void checkCookies(KHttpCookie *cookie, bool queue);

public slots:
  /**
   * Save the cookiejar.
   */
  void slotSave();

protected:
  KCookieJar *mCookieJar;
  KHttpCookieList *mPendingCookies;
  RequestList *mRequestList;
  QTimer *mTimer;
  bool mAdvicePending;
};

#endif
