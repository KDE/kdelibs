/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte.starostik@t-online.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _KSSLINFODLG_H
#define _KSSLINFODLG_H

#include <kdialog.h>

#include "ksslx509map.h"
#include "ksslcertificate.h"
#include "kssl.h"

class QScrollView;
class QWidget;

class KSSLInfoDlg : public KDialog {

  Q_OBJECT

public:
  KSSLInfoDlg(bool secureConnection, QWidget *parent = 0, const char *name = 0, bool modal = false);
  virtual ~KSSLInfoDlg();

  void setup(const QString& peername, const QString& issuer,
             const QString& ip, const QString& url,
             const QString& cipher, const QString& cipherdesc,
             const QString& sslversion, int usedbits, int bits,
             KSSLCertificate::KSSLValidation certState,
             const QString& goodFrom, const QString& goodUntil);

  void setup( KSSL & ssl, const QString & ip, const QString & url );
  static QScrollView *certInfoWidget(QWidget *parent, const QString &certName, bool doMail = false);

private:
  QScrollView *buildCertInfo(const QString &certName);

private:
  class KSSLInfoDlgPrivate;
  KSSLInfoDlgPrivate *d;

private slots:
  void launchConfig();
  void urlClicked(const QString &url);
  void mailClicked(const QString &url);

};

#endif

