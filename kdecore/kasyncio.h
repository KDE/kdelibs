/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Thiago Macieira <thiago.macieira@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KASYNCIO_H
#define KASYNCIO_H


#include <qobject.h>
#include <qiodevice.h>
#include "kdelibs_export.h"

class KAsyncIOPrivate;
/**
 * Asynchronous I/O Support
 *
 * This abstract class provides basic functionality for asynchronous I/O
 * support on top of QIODevice.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 * @short Asynchronous I/O support
 */
class KDECORE_EXPORT KAsyncIO: public QObject, public QIODevice
{
  Q_OBJECT
protected:
  KAsyncIO()			// cannot be accessed externally
  { }

private:
  KAsyncIO(KAsyncIO&);

  KAsyncIO& operator=(KAsyncIO&);

public:
  /**
   * Toggles the emission of the readyRead() signal whenever the device
   * is ready for reading. This is useful if you want to know the first time
   * the device is ready for reading and you don't want to read it now.
   * @param enable true to enable, false to disable the readyRead() signal
   */
  virtual void enableRead(bool enable) = 0;

  /**
   * Toggles the emission of the readyWrite() signal whenever the device
   * is ready for writing. This is useful if you want to know the first time
   * the device is ready for writing and you don't want to write to it now.
   * @param enable true to enable, false to disable the readyWrite() signal
   */
  virtual void enableWrite(bool enable) = 0;

signals:

  /**
   * This signal gets sent when the device is ready for reading.
   */
  void readyRead();

  /**
   * This signal gets sent when the device is ready for writing.
   */
  void readyWrite();
protected:
    /** \internal */
  virtual void virtual_hook( int id, void* data );
private:
  KAsyncIOPrivate* d;
};

#endif // KASYNCIO_H
