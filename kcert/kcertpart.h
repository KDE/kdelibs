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
#ifndef _INCLUDE_KCERTPART_H
#define _INCLUDE_KCERTPART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kparts/part.h>

class KSSLCertBox;
class QFrame;
class QLabel;
class KCertPartPrivate;
class QComboBox;
class QButton;
class KSSLPKCS12;
class KSSLCertificate;
class QTabWidget;
class QMultiLineEdit;



class KCertPart : public KParts::ReadWritePart {
Q_OBJECT
public:
  KCertPart(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KCertPart();

  virtual void setReadWrite(bool rw);


protected slots:
  void slotChain(int c);
  void slotImport();
  void slotSave();
  void slotDone();
  void slotLaunch();

protected:

  virtual bool openFile();
  virtual bool saveFile();

  void displayCert(KSSLCertificate *c);

  QFrame *_frame;
  QLabel *_filenameLabel, *_validFrom, *_validUntil, *_serialNum, *_certState;
  QLabel *_digest;
  QMultiLineEdit *_pubkey, *_sig;
  KSSLCertBox *_subject, *_issuer;
  KInstance *_instance;
  QComboBox *_chain;
  QButton *_import, *_save, *_done, *_launch;
  KSSLPKCS12 *_p12;
  QTabWidget *_tabs;

private:
  KCertPartPrivate *d;
};




#endif


