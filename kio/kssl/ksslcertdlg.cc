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
#include <qlistview.h>
#include <qframe.h>
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
    QLabel *p_message;
    QPushButton *p_pb_dontsend;
    bool p_send_flag;
};

KSSLCertDlg::KSSLCertDlg(QWidget *parent, const char *name, bool modal)
 : KDialog(parent, name, modal), d(new KSSLCertDlgPrivate) {

   QBoxLayout * grid = new QVBoxLayout( this, KDialog::marginHint(),
                                              KDialog::spacingHint() );

   d->p_message = new QLabel(QString::null, this);
   grid->addWidget(d->p_message);
   setHost(_host);

   _certs = new QListView(this);
   _certs->addColumn(i18n("Certificate"));
   _certs->setResizeMode(QListView::LastColumn);
   QFontMetrics fm( KGlobalSettings::generalFont() );
   _certs->setMinimumHeight(4*fm.height());
   grid->addWidget(_certs);

   _save = new QCheckBox(i18n("Save selection for this host."), this);
   grid->addWidget(_save);

   grid->addWidget(new KSeparator(KSeparator::HLine, this));

   QBoxLayout * h = new QHBoxLayout( grid );
   h->insertStretch(0);

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


KSSLCertDlg::~KSSLCertDlg() {
    delete d;
}


void KSSLCertDlg::setup(QStringList certs, bool saveChecked, bool sendChecked) {
	setupDialog(certs, saveChecked, sendChecked);
}

void KSSLCertDlg::setupDialog(const QStringList& certs, bool saveChecked, bool sendChecked) {
  _save->setChecked(saveChecked);
  d->p_send_flag = sendChecked;

  if (sendChecked)
    _ok->setDefault(true); // "do send" is the "default action".
  else
    d->p_pb_dontsend->setDefault(true); // "do not send" is the "default action".

  for (QStringList::ConstIterator i = certs.begin(); i != certs.end(); ++i) {
    if ((*i).isEmpty())
      continue;

    new QListViewItem(_certs, *i);
  }

  _certs->setSelected(_certs->firstChild(), true);
}


bool KSSLCertDlg::saveChoice() {
  return _save->isChecked();
}


bool KSSLCertDlg::wantsToSend() {
  return d->p_send_flag;
}


QString KSSLCertDlg::getChoice() {
   QListViewItem *selected = _certs->selectedItem();
   if (selected && d->p_send_flag)
	return selected->text(0);
   else
	return QString::null;
}


void KSSLCertDlg::setHost(const QString& host) {
   _host = host;
   d->p_message->setText(i18n("The server <b>%1</b> requests a certificate.<p>"
			     "Select a certificate to use from the list below:")
			 .arg(_host));
}


void KSSLCertDlg::slotSend() {
   d->p_send_flag = true;
   accept();
}


void KSSLCertDlg::slotDont() {
   d->p_send_flag = false;
   reject();
}


QDataStream& operator<<(QDataStream& s, const KSSLCertDlgRet& r) {
   s << Q_INT8(r.ok?1:0) <<  r.choice << Q_INT8(r.save?1:0) << Q_INT8(r.send?1:0);
   return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertDlgRet& r) {
Q_INT8 tmp;
   s >> tmp; r.ok = (tmp == 1);
   s >> r.choice;
   s >> tmp; r.save = (tmp == 1);
   s >> tmp; r.send = (tmp == 1);
   return s;
}


#include "ksslcertdlg.moc"

