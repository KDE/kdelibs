/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
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
#include <qscrollview.h>

class QWidget;
class KSSLCertBox;
class KSSLCertChain;


//  NO GUARANTEES THAT KSSLInfoDlg:: will remain BC
//  Contact staikos@kde.org for details if needed

class KSSLInfoDlg : public KDialog {

  Q_OBJECT

public:
  KSSLInfoDlg(bool secureConnection, QWidget *parent = 0, const char *name = 0, bool modal = false);
  virtual ~KSSLInfoDlg();

  void setup(KSSLCertificate *cert,
             const QString& ip, const QString& url,
             const QString& cipher, const QString& cipherdesc,
             const QString& sslversion, int usedbits, int bits,
             KSSLCertificate::KSSLValidation certState);

public:
  void setup( KSSL & ssl, const QString & ip, const QString & url );
  static KSSLCertBox *certInfoWidget(QWidget *parent, const QString &certName, QWidget *mailCatcher=0);

private:
  QScrollView *buildCertInfo(const QString &certName);
  void displayCert(KSSLCertificate *x);

  class KSSLInfoDlgPrivate;
  KSSLInfoDlgPrivate *d;

private slots:
  void launchConfig();
  void urlClicked(const QString &url);
  void mailClicked(const QString &url);
  void slotChain(int x);
};


class KSSLCertBox : public QScrollView {
public:
   KSSLCertBox(QWidget *parent=0, const char *name=0, WFlags f=0);
   void setValues(QString certName, QWidget *mailCatcher=0);
private:
   QFrame *_frame;
};

#endif

