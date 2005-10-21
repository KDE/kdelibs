/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ksslcertdlg.h"

#include <kssl.h>

#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <q3listview.h>
#include <q3frame.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kseparator.h>
#include <kdebug.h>


class KSSLCertDlg::KSSLCertDlgPrivate {
private:
    friend class KSSLCertDlg;
};

KSSLCertDlg::KSSLCertDlg(QWidget *parent, const char *name, bool modal)
 : KDialog(parent, name, modal), d(new KSSLCertDlgPrivate) {
   QGridLayout *grid = new QGridLayout(this, 8, 6, KDialog::marginHint(),
                                                   KDialog::spacingHint() );

   _send = new QRadioButton(i18n("Send certificate..."), this);
   grid->addMultiCellWidget(_send, 0, 0, 0, 2);
   connect(_send, SIGNAL(clicked()), SLOT(slotSend()));

   _dont = new QRadioButton(i18n("Do not send a certificate"), this);
   grid->addMultiCellWidget(_dont, 1, 1, 0, 2);
   connect(_dont, SIGNAL(clicked()), SLOT(slotDont()));

   _certs = new Q3ListView(this);
   grid->addMultiCellWidget(_certs, 0, 4, 3, 5);
   _certs->addColumn(i18n("Certificate"));

   _save = new QCheckBox(i18n("Save selection for this host."), this);
   grid->addMultiCellWidget(_save, 5, 5, 0, 3);

   grid->addMultiCellWidget(new KSeparator(Qt::Horizontal, this), 6, 6, 0, 5);

   _ok = new KPushButton(KStdGuiItem::cont(), this);
   grid->addWidget(_ok, 7, 5);
   connect(_ok, SIGNAL(clicked()), SLOT(accept()));

#ifndef QT_NO_WIDGET_TOPEXTRA
   setCaption(i18n("KDE SSL Certificate Dialog"));
#endif
}


KSSLCertDlg::~KSSLCertDlg() {
    delete d;
}


void KSSLCertDlg::setup(QStringList certs, bool saveChecked, bool sendChecked) {
	setupDialog(certs, saveChecked, sendChecked);
}

void KSSLCertDlg::setupDialog(const QStringList& certs, bool saveChecked, bool sendChecked) {
  _save->setChecked(saveChecked);
  _send->setChecked(sendChecked);
  _dont->setChecked(!sendChecked);
  _certs->setEnabled(saveChecked);

  for (QStringList::ConstIterator i = certs.begin(); i != certs.end(); ++i) {
    if ((*i).isEmpty())
      continue;

    new Q3ListViewItem(_certs, *i);
  }

  _certs->setSelected(_certs->firstChild(), true);
}


bool KSSLCertDlg::saveChoice() {
  return _save->isChecked();
}


bool KSSLCertDlg::wantsToSend() {
  return _send->isChecked();
}


QString KSSLCertDlg::getChoice() {
   return _certs->selectedItem()->text(0);
}


void KSSLCertDlg::setHost(const QString& host) {
   _host = host;
#ifndef QT_NO_WIDGET_TOPEXTRA
   setCaption(i18n("KDE SSL Certificate Dialog")+" - "+host);
#endif
}


void KSSLCertDlg::slotSend() {
   _dont->setChecked(false);
   _send->setChecked(true);
   _certs->setEnabled(true);
}


void KSSLCertDlg::slotDont() {
   _send->setChecked(false);
   _dont->setChecked(true);
   _certs->setEnabled(false);
}


QDataStream& operator<<(QDataStream& s, const KSSLCertDlgRet& r) {
   s << qint8(r.ok?1:0) <<  r.choice << qint8(r.save?1:0) << qint8(r.send?1:0);
   return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertDlgRet& r) {
qint8 tmp;
   s >> tmp; r.ok = (tmp == 1);
   s >> r.choice;
   s >> tmp; r.save = (tmp == 1);
   s >> tmp; r.send = (tmp == 1);
   return s;
}


#include "ksslcertdlg.moc"

