/*
   Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License (LGPL)
   version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

   RFC 1321 "MD5 Message-Digest Algorithm" Copyright (C) 1991-1992.             // krazy:exclude=copyright
   RSA Data Security, Inc. Created 1991. All rights reserved.

   The KMD5 class is based on a C++ implementation of
   "RSA Data Security, Inc. MD5 Message-Digest Algorithm" by
   Mordechai T. Abzug,	Copyright (c) 1995.  This implementation                // krazy:exclude=copyright
   passes the test-suite as defined in RFC 1321.

   The encoding and decoding utilities in KCodecs with the exception of
   quoted-printable are based on the java implementation in HTTPClient
   package by Ronald Tschalär Copyright (C) 1996-1999.                          // krazy:exclude=copyright

   The quoted-printable codec as described in RFC 2045, section 6.7. is by
   Rik Hemsley (C) 2001.
*/

#ifndef KMD5_H
#define KMD5_H

#include <kde4support_export.h>
#include <qglobal.h>

class QByteArray;
class QIODevice;

class KMD5Private;
/**
 * @short An adapted C++ implementation of RSA Data Securities MD5 algorithm.
 *
 * DEPRECATED. please use QCryptographicHash instead
 *
 * The default constructor is designed to provide much the same
 * functionality as the most commonly used C-implementation, while
 * the other three constructors are meant to further simplify the
 * process of obtaining a digest by calculating the result in a
 * single step.
 *
 * KMD5 is state-based, that means you can add new contents with
 * update() as long as you didn't request the digest value yet.
 * After the digest value was requested, the object is "finalized"
 * and you have to call reset() to be able to do another calculation
 * with it.  The reason for this behavior is that upon requesting
 * the message digest KMD5 has to pad the received contents up to a
 * 64 byte boundary to calculate its value. After this operation it
 * is not possible to resume consuming data.
 *
 * \b Usage:
 *
 * A common usage of this class:
 *
 * \code
 * const char* test1;
 * KMD5::Digest rawResult;
 *
 * test1 = "This is a simple test.";
 * KMD5 context (test1);
 * cout << "Hex Digest output: " << context.hexDigest().data() << endl;
 * \endcode
 *
 * To cut down on the unnecessary overhead of creating multiple KMD5
 * objects, you can simply invoke reset() to reuse the same object
 * in making another calculation:
 *
 * \code
 * context.reset ();
 * context.update ("TWO");
 * context.update ("THREE");
 * cout << "Hex Digest output: " << context.hexDigest().data() << endl;
 * \endcode
 *
 * @author Dirk Mueller <mueller@kde.org>, Dawit Alemayehu <adawit@kde.org>
 *
 * @deprecated
 * @see QCryptographicHash
 */

class KDE4SUPPORT_DEPRECATED_EXPORT KMD5
{
public:

  typedef unsigned char Digest[16];

  KMD5();
  ~KMD5();

  /**
   * Constructor that updates the digest for the given string.
   *
   * @param in   C string or binary data
   * @param len  if negative, calculates the length by using
   *             strlen on the first parameter, otherwise
   *             it trusts the given length (does not stop on NUL byte).
   */
  explicit KMD5(const char* in, int len = -1);

  /**
   * @overload
   *
   * Same as above except it accepts a QByteArray as its argument.
   */
  explicit KMD5(const QByteArray& a );

  /**
   * Updates the message to be digested. Be sure to add all data
   * before you read the digest. After reading the digest, you
   * can <b>not</b> add more data!
   *
   * @param in     message to be added to digest
   * @param len    the length of the given message.
   *
   * @deprecated please use QCryptographicHash::addData instead
   */
  void update(const char* in, int len = -1);

  /**
   * @overload
   *
   * please use QCryptographicHash::addData instead
   */
  void update(const unsigned char* in, int len = -1);

  /**
   * @overload
   *
   * @param in     message to be added to the digest (QByteArray).
   *
   * @deprecated please use QCryptographicHash::addData instead
   */
  void update(const QByteArray& in );

  /**
   * @overload
   *
   * reads the data from an I/O device, i.e. from a file (QFile).
   *
   * NOTE that the file must be open for reading.
   *
   * @param file       a pointer to FILE as returned by calls like f{d,re}open
   *
   * @returns false if an error occurred during reading.
   *
   * @deprecated please use QCryptographicHash::addData instead
   */
  bool update(QIODevice& file);

  /**
   * Calling this function will reset the calculated message digest.
   * Use this method to perform another message digest calculation
   * without recreating the KMD5 object.
   *
   * @deprecated please use QCryptographicHash::reset() instead
   */
  void reset();

  /**
   * @return the raw representation of the digest
   * @deprecated please use QCryptographicHash::result instead
   */
  const Digest& rawDigest (); //krazy:exclude=constref (simple array)

  /**
   * Fills the given array with the binary representation of the
   * message digest.
   *
   * Use this method if you do not want to worry about making
   * copy of the digest once you obtain it.
   *
   * @param bin an array of 16 characters ( char[16] )
   */
  void rawDigest( KMD5::Digest& bin );

  /**
   * Returns the value of the calculated message digest in
   * a hexadecimal representation.
   * @deprecated please use QCryptographicHash::result().toHex() instead
   */
  QByteArray hexDigest ();

  /**
   * @overload
   */
  void hexDigest(QByteArray&);

  /**
   * Returns the value of the calculated message digest in
   * a base64-encoded representation.
   * @deprecated please use QCryptographicHash::result().toBase64() instead
   */
  QByteArray base64Digest ();

  /**
   * returns true if the calculated digest for the given
   * message matches the given one.
   */
  bool verify( const KMD5::Digest& digest);

  /**
   * @overload
   */
  bool verify(const QByteArray&);

protected:
  /**
   *  Performs the real update work.  Note
   *  that length is implied to be 64.
   */
  void transform( const unsigned char buffer[64] );

  /**
   * finalizes the digest
   */
  void finalize();

private:
  KMD5(const KMD5& u);
  KMD5& operator=(const KMD5& md);

  void init();
  void encode( unsigned char* output, quint32 *in, quint32 len );
  void decode( quint32 *output, const unsigned char* in, quint32 len );

  quint32 rotate_left( quint32 x, quint32 n );
  quint32 F( quint32 x, quint32 y, quint32 z );
  quint32 G( quint32 x, quint32 y, quint32 z );
  quint32 H( quint32 x, quint32 y, quint32 z );
  quint32 I( quint32 x, quint32 y, quint32 z );
  void FF( quint32& a, quint32 b, quint32 c, quint32 d, quint32 x,
               quint32  s, quint32 ac );
  void GG( quint32& a, quint32 b, quint32 c, quint32 d, quint32 x,
                quint32 s, quint32 ac );
  void HH( quint32& a, quint32 b, quint32 c, quint32 d, quint32 x,
                quint32 s, quint32 ac );
  void II( quint32& a, quint32 b, quint32 c, quint32 d, quint32 x,
             quint32 s, quint32 ac );

private:
  quint32 m_state[4];
  quint32 m_count[2];
  quint8 m_buffer[64];
  Digest m_digest;
  bool m_finalized;

  KMD5Private* d;
};


#endif // KCODECS_H
