/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat < >
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


#ifndef _KCARDIMPLEMENTATION_H
#define _KCARDIMPLEMENTATION_H

#include <qstring.h>
#include <kpcsc.h>

#define KCARD_TYPE_UNKNOWN   "Unknown"
#define KCARD_TYPE_PROCESSOR "Processor"
#define KCARD_TYPE_GSM       "GSM"
#define KCARD_TYPE_MEMORY    "Memory"


class KCardReader;

class KCardImplementation {


 public:

  KCardImplementation (KCardReader *);
  virtual ~KCardImplementation();

  virtual int selectFile(const QString);
  virtual int selectDirectory(const QString);
  virtual KCardCommand getCardSerialNumber();

  virtual const QString& getType() const;
  virtual const QString& getSubType() const;
  virtual const QString& getSubSubType() const;
  
 private:

  class KCardImplementationPrivate;
  KCardImplementationPrivate *d;

 protected:

  KCardReader * _kcardreader;
  QString _type, _subType, _subSubType;
};

#endif
