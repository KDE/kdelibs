/**
 * certexport.h
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _CERTEXPORT_H
#define _CERTEXPORT_H

#include <kdialog.h>

class KLineEdit;
class QRadioButton;
class QPushButton;
class KSSLCertificate;


class KCertExport : public KDialog
{
  Q_OBJECT
public:
  KCertExport(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KCertExport();

  void setCertificate(KSSLCertificate *c);

protected Q_SLOTS:
  void slotExport();
  void slotChoose();
  void slotTextChanged(const QString &);

private:
  QPushButton *_export, *_cancel, *_choose;
  QRadioButton *_pem, *_netscape, *_der, *_text;
  KLineEdit *_filename;

  KSSLCertificate *_c;

};

#endif
