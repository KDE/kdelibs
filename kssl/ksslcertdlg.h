/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCERTDLG_H
#define _KSSLCERTDLG_H

#include <kdialog.h>
#include <qstringlist.h>

class QWidget;
class QCheckBox;
class QRadioButton;
class QListView;
class QPushButton;

class KSSLCertDlg : public KDialog {

  Q_OBJECT

public:
  KSSLCertDlg(QWidget *parent = 0, const char *name = 0, bool modal = false);
  virtual ~KSSLCertDlg();

  void setup(QStringList certs, bool saveChecked = false, bool sendChecked = true);

  QString getChoice();

  bool wantsToSend();

  bool saveChoice();

  void setHost(const QString& host);

private slots:
  void slotSend();
  void slotDont();


private:
  class KSSLCertDlgPrivate;
  KSSLCertDlgPrivate *d;
  QCheckBox *_save;
  QRadioButton *_send, *_dont;
  QListView *_certs;
  QPushButton *_ok;
  QString _host;

};


class KSSLCertDlgRet {
public:
   bool ok;
   QString choice;
   bool send;
   bool save;

protected:
   class KSSLCertDlgRetPrivate;
   KSSLCertDlgRetPrivate *d;
};

QDataStream& operator<<(QDataStream& s, const KSSLCertDlgRet& r);
QDataStream& operator>>(QDataStream& s, KSSLCertDlgRet& r);




#endif

