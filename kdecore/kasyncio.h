/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#ifndef KASYNCIO_H
#define KASYNCIO_H


#include <qobject.h>
#include <qiodevice.h>

class KAsyncIOPrivate;
/**
 * Asynchronous I/O Support
 *
 * This abstract class provides basic functionality for asynchronous I/O
 * support.
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short asynchronous I/O support
 */
class KAsyncIO: public QObject, public QIODevice
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
   * Toggles the emission of the readyRead signal whenever the device
   * is ready for reading. This is useful if you want to know the first time
   * the device is ready for reading and you don't want to read it now.
   */
  virtual void enableRead(bool enable) = 0;

  /**
   * Ditto for readyWrite
   */
  virtual void enableWrite(bool enable) = 0;

signals:

  /**
   * This signal gets sent when the device is ready for reading
   */
  void readyRead();

  /**
   * This signal gets sent when the device is ready for writing
   */
  void readyWrite();
protected:
    /// @internal
  virtual void virtual_hook( int id, void* data );
private:
  KAsyncIOPrivate* d;
};

#endif // KASYNCIO_H
