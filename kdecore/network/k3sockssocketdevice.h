/*  -*- C++ -*-
 *  Copyright (C) 2004,2005 Thiago Macieira <thiago@kde.org>
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

#ifndef KSOCKSSOCKETDEVICE_H
#define KSOCKSSOCKETDEVICE_H

#include "k3socketdevice.h"

namespace KNetwork {

class KSocksSocketDevicePrivate;
/** 
 * @class KSocksSocketDevice k3sockssocketdevice.h k3sockssocketdevice.h
 * @brief The low-level class for SOCKS proxying.
 *
 * This class reimplements several functions from KSocketDevice in order
 * to implement SOCKS support.
 *
 * This works by using KSocks.
 *
 * @author Thiago Macieira <thiago@kde.org>
 *
 * @warning This code is untested!
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KDECORE_EXPORT KSocksSocketDevice: public KSocketDevice
{
public:
  /** 
   * Constructor.
   */
  KSocksSocketDevice(const KSocketBase* = 0L);

  /**
   * Construct from a file descriptor.
   */
  explicit KSocksSocketDevice(int fd);

  /**
   * Destructor.
   */
  virtual ~KSocksSocketDevice();

  /**
   * Sets our capabilities.
   */
  virtual int capabilities() const;

  /**
   * Overrides binding.
   */
  virtual bool bind(const KResolverEntry& address);

  /**
   * Overrides listening.
   */
  virtual bool listen(int backlog);

  /**
   * Overrides connection.
   */
  virtual bool connect(const KResolverEntry& address);

  /**
   * Overrides accepting. The return type is specialized.
   */
  virtual KSocksSocketDevice* accept();

  /**
   * Overrides reading.
   */
  virtual qint64 readBlock(char *data, quint64 maxlen);

  /**
   * Overrides reading.
   *
   * @todo (Thiago, you saw this coming) Actually document this.
   *       What is the @p from for?
   */
  virtual qint64 readBlock(char *data, quint64 maxlen, KSocketAddress& from);

  /**
   * Overrides peeking.
   */
  virtual qint64 peekBlock(char *data, quint64 maxlen);

  /**
   * Overrides peeking.
   */
  virtual qint64 peekBlock(char *data, quint64 maxlen, KSocketAddress& from);

  /**
   * Overrides writing.
   */
  virtual qint64 writeBlock(const char *data, quint64 len);

  /**
   * Overrides writing.
   */
  virtual qint64 writeBlock(const char *data, quint64 len, const KSocketAddress& to);

  /**
   * Overrides getting socket address.
   */
  virtual KSocketAddress localAddress() const;

  /**
   * Overrides getting peer address.
   */
  virtual KSocketAddress peerAddress() const;

  /**
   * Overrides getting external address.
   */
  virtual KSocketAddress externalAddress() const;

  /**
   * Overrides polling.
   */
  virtual bool poll(bool* input, bool* output, bool* exception = 0L,
		    int timeout = -1, bool* timedout = 0L);

private:
  static void initSocks();
  friend class KSocketDevice;
  KSocksSocketDevicePrivate * const d;
};

}				// namespace KNetwork

#endif
