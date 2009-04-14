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

#include "ksslcertdialog.h"

#include <kssl.h>

#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>
#include <kseparator.h>
#include <kdebug.h>


class KSSLCertDialog::KSSLCertDialogPrivate {
private:
    friend class KSSLCertDialog;
    QLabel *p_message;
    QPushButton *p_pb_dontsend;
    bool p_send_flag;
};

KSSLCertDialog::KSSLCertDialog(QWidget *parent, const char *name, bool modal)
 : KDialog(parent), d(new KSSLCertDialogPrivate) {
   setObjectName(name);
   setModal(modal);

   QBoxLayout * grid = new QVBoxLayout( this );

   d->p_message = new QLabel(QString(), this);
   grid->addWidget(d->p_message);
   setHost(_host);

   QLabel* lblCertificate = new QLabel(i18n("Certificate"));
   grid->addWidget(lblCertificate);

   _certs = new QListWidget(this);
   QFontMetrics fm( KGlobalSettings::generalFont() );
   _certs->setMinimumHeight(4*fm.height());
   grid->addWidget(_certs);

   _save = new QCheckBox(i18n("Save selection for this host."), this);
   grid->addWidget(_save);

   grid->addWidget(new KSeparator(Qt::Horizontal, this));

   QBoxLayout * h = new QHBoxLayout(this);
   h->insertStretch(0);
   grid->addLayout(h);

   _ok = new KPushButton(i18n("Send certificate"), this);
   h->addWidget(_ok);
   connect(_ok, SIGNAL(clicked()), SLOT(slotSend()));

   d->p_pb_dontsend = new KPushButton(i18n("Do not send a certificate"), this);
   h->addWidget(d->p_pb_dontsend);
   connect(d->p_pb_dontsend, SIGNAL(clicked()), SLOT(slotDont()));

#ifndef QT_NO_WIDGET_TOPEXTRA
   setCaption(i18n("KDE SSL Certificate Dialog"));
#endif
}


KSSLCertDialog::~KSSLCertDialog() {
    delete d;
}


void KSSLCertDialog::setup(QStringList certs, bool saveChecked, bool sendChecked) {
	setupDialog(certs, saveChecked, sendChecked);
}

void KSSLCertDialog::setupDialog(const QStringList& certs, bool saveChecked, bool sendChecked) {
  _save->setChecked(saveChecked);
  d->p_send_flag = sendChecked;

  if (sendChecked)
    _ok->setDefault(true); // "do send" is the "default action".
  else
    d->p_pb_dontsend->setDefault(true); // "do not send" is the "default action".

  for (QStringList::ConstIterator i = certs.begin(); i != certs.end(); ++i) {
    if ((*i).isEmpty())
      continue;

    new QListWidgetItem(*i, _certs);
  }

  _certs->setCurrentItem(_certs->item(0));
}


bool KSSLCertDialog::saveChoice() {
  return _save->isChecked();
}


bool KSSLCertDialog::wantsToSend() {
  return d->p_send_flag;
}


QString KSSLCertDialog::getChoice() {
   QListWidgetItem *selected = _certs->currentItem();
   if (selected && d->p_send_flag)
	return selected->text();
   else
	return QString();
}


void KSSLCertDialog::setHost(const QString& host) {
   _host = host;
   d->p_message->setText(i18n("The server <b>%1</b> requests a certificate.<br /><br />"
			      "Select a certificate to use from the list below:",
			      _host));
}


void KSSLCertDialog::slotSend() {
   d->p_send_flag = true;
   accept();
}


void KSSLCertDialog::slotDont() {
   d->p_send_flag = false;
   reject();
}


QDataStream& operator<<(QDataStream& s, const KSSLCertDialogRet& r) {
   s << qint8(r.ok?1:0) <<  r.choice << qint8(r.save?1:0) << qint8(r.send?1:0);
   return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertDialogRet& r) {
   qint8 tmp;
   s >> tmp; r.ok = (tmp == 1);
   s >> r.choice;
   s >> tmp; r.save = (tmp == 1);
   s >> tmp; r.send = (tmp == 1);
   return s;
}


#include "ksslcertdialog.moc"

