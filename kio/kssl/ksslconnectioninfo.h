/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCONNECTIONINFO_H
#define _KSSLCONNECTIONINFO_H

#include <qstring.h>

class KSSL;

class KSSLConnectionInfo {
friend class KSSL;
public:
  ~KSSLConnectionInfo();

  const QString& getCipher() const;
  const QString& getCipherDescription() const;
  const QString& getCipherVersion() const;
  int getCipherUsedBits() const;
  int getCipherBits() const;

private:

protected:
  KSSLConnectionInfo();
  void clean();

  // These are here so KSSL can access them directly
  // It's just as easy as making accessors - they're friends afterall!
  int m_iCipherUsedBits, m_iCipherBits;
  QString m_cipherName;
  QString m_cipherDescription;
  QString m_cipherVersion;

private:
  class KSSLConnectionInfoPrivate;
  KSSLConnectionInfoPrivate *d;
};

#endif

