/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat < >
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


#ifndef __KCARD_GSM_TYPE_H
#define __KCARD_GSM_TYPE_H

#include <qstringlist.h>

#include "kcardimplementation.h"

class KCardGsmImplementation:public KCardImplementation{


 public:

  KCardGsmImplementation (KCardReader *);
  ~KCardGsmImplementation();

  int selectFile(const QString);
  int readTransparentFile   (QString &);
  int readTransparentFile   (QString &, unsigned char offset, unsigned char length);
  int updateTransparentFile (QString &, unsigned char offset, unsigned char length);

  int readLinearFixedFile    (QStringList&);
  int readLinearFixedFileRecord    (QStringList&, unsigned char record);

  int readCyclicFile (QStringList &);
  
  int verifyCHV1  (const QString &);
  int verifyCHV2  (const QString &);
  int verifyUnblockCHV1 (const QString &);
  int verifyUnblockCHV2 (const QString &);

  int runGsmAlgorithm(const QString &, QString &);


 private:

  class  KCardGSMImplementationPrivate;
  KCardGSMImplementationPrivate *d;

};

#endif

