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

class KCookieList;
class KCookieJar;
class KCookie;
class QTimer;

class KCookieServer : public KUniqueApplication
{
  Q_OBJECT 
public:
  KCookieServer(int argc, char *argv[]);
  ~KCookieServer();

  virtual bool process(const QCString &fun, const QByteArray &data,
		       QCString& replyType, QByteArray &replyData);

  void addCookies(QString url, QCString cookieHeader);
  void checkCookies(KCookie *cookie, bool queue);

public slots:
  /**
   * Save the cookiejar.
   */
  void slotSave();

protected:
  KCookieJar *mCookieJar;
  KCookieList *mPendingCookies;
  QTimer *mTimer;
  bool mAdvicePending;
};

#endif
