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

#include "kmd5.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QTextCodec>

#define KMD5_S11 7
#define KMD5_S12 12
#define KMD5_S13 17
#define KMD5_S14 22
#define KMD5_S21 5
#define KMD5_S22 9
#define KMD5_S23 14
#define KMD5_S24 20
#define KMD5_S31 4
#define KMD5_S32 11
#define KMD5_S33 16
#define KMD5_S34 23
#define KMD5_S41 6
#define KMD5_S42 10
#define KMD5_S43 15
#define KMD5_S44 21

KMD5::KMD5()
{
    init();
}

KMD5::KMD5(const char *in, int len)
{
    init();
    update(in, len);
}

KMD5::KMD5(const QByteArray& in)
{
    init();
    update( in );
}

KMD5::~KMD5()
{
}

void KMD5::update(const QByteArray& in)
{
    update(in.data(), int(in.size()));
}

void KMD5::update(const char* in, int len)
{
    update(reinterpret_cast<const unsigned char*>(in), len);
}

void KMD5::update(const unsigned char* in, int len)
{
    if (len < 0)
        len = qstrlen(reinterpret_cast<const char*>(in));

    if (!len)
        return;

    if (m_finalized) {
        qWarning() << "KMD5::update called after state was finalized!";
        return;
    }

    quint32 in_index;
    quint32 buffer_index;
    quint32 buffer_space;
    quint32 in_length = static_cast<quint32>( len );

    buffer_index = static_cast<quint32>((m_count[0] >> 3) & 0x3F);

    if (  (m_count[0] += (in_length << 3))<(in_length << 3) )
        m_count[1]++;

    m_count[1] += (in_length >> 29);
    buffer_space = 64 - buffer_index;

    if (in_length >= buffer_space)
    {
        memcpy (m_buffer + buffer_index, in, buffer_space);
        transform (m_buffer);

        for (in_index = buffer_space; in_index + 63 < in_length;
             in_index += 64)
            transform (reinterpret_cast<const unsigned char*>(in+in_index));

        buffer_index = 0;
    }
    else
        in_index=0;

    memcpy(m_buffer+buffer_index, in+in_index, in_length-in_index);
}

bool KMD5::update(QIODevice& file)
{
    char buffer[1024];
    int len;

    while ((len=file.read(buffer, sizeof(buffer))) > 0)
        update(buffer, len);

    return file.atEnd();
}

void KMD5::finalize ()
{
    if (m_finalized) return;

    quint8 bits[8];
    quint32 index, padLen;
    static const unsigned char PADDING[64]=
    {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    encode (bits, m_count, 8);
    //memcpy( bits, m_count, 8 );

    // Pad out to 56 mod 64.
    index = static_cast<quint32>((m_count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    update (reinterpret_cast<const char*>(PADDING), padLen);

    // Append length (before padding)
    update (reinterpret_cast<const char*>(bits), 8);

    // Store state in digest
    encode (m_digest, m_state, 16);
    //memcpy( m_digest, m_state, 16 );

    // Fill sensitive information with zero's
    memset ( (void *)m_buffer, 0, sizeof(*m_buffer));

    m_finalized = true;
}


bool KMD5::verify( const KMD5::Digest& digest)
{
    finalize();
    return (0 == memcmp(rawDigest(), digest, sizeof(KMD5::Digest)));
}

bool KMD5::verify( const QByteArray& hexdigest)
{
    finalize();
    return (0 == strcmp(hexDigest().data(), hexdigest.data()));
}

const KMD5::Digest& KMD5::rawDigest()
{
    finalize();
    return m_digest;
}

void KMD5::rawDigest( KMD5::Digest& bin )
{
    finalize();
    memcpy( bin, m_digest, 16 );
}


QByteArray KMD5::hexDigest()
{
    QByteArray s(32, 0);

    finalize();
    sprintf(s.data(), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            m_digest[0], m_digest[1], m_digest[2], m_digest[3], m_digest[4], m_digest[5],
            m_digest[6], m_digest[7], m_digest[8], m_digest[9], m_digest[10], m_digest[11],
            m_digest[12], m_digest[13], m_digest[14], m_digest[15]);

    return s;
}

void KMD5::hexDigest(QByteArray& s)
{
    finalize();
    s.resize(32);
    sprintf(s.data(), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            m_digest[0], m_digest[1], m_digest[2], m_digest[3], m_digest[4], m_digest[5],
            m_digest[6], m_digest[7], m_digest[8], m_digest[9], m_digest[10], m_digest[11],
            m_digest[12], m_digest[13], m_digest[14], m_digest[15]);
}

QByteArray KMD5::base64Digest()
{
    finalize();
    return QByteArray::fromRawData(reinterpret_cast<const char*>(m_digest),16).toBase64();
}

void KMD5::init()
{
    d = 0;
    reset();
}

void KMD5::reset()
{
    m_finalized = false;

    m_count[0] = 0;
    m_count[1] = 0;

    m_state[0] = 0x67452301;
    m_state[1] = 0xefcdab89;
    m_state[2] = 0x98badcfe;
    m_state[3] = 0x10325476;

    memset ( m_buffer, 0, sizeof(*m_buffer));
    memset ( m_digest, 0, sizeof(*m_digest));
}

void KMD5::transform( const unsigned char block[64] )
{

    quint32 a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3], x[16];

    decode (x, block, 64);
    //memcpy( x, block, 64 );

    Q_ASSERT(!m_finalized);  // not just a user error, since the method is private

    /* Round 1 */
    FF (a, b, c, d, x[ 0], KMD5_S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], KMD5_S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], KMD5_S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], KMD5_S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], KMD5_S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], KMD5_S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], KMD5_S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], KMD5_S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], KMD5_S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], KMD5_S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], KMD5_S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], KMD5_S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], KMD5_S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], KMD5_S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], KMD5_S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], KMD5_S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG (a, b, c, d, x[ 1], KMD5_S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], KMD5_S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], KMD5_S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], KMD5_S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], KMD5_S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], KMD5_S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], KMD5_S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], KMD5_S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], KMD5_S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], KMD5_S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], KMD5_S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], KMD5_S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], KMD5_S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], KMD5_S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], KMD5_S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], KMD5_S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH (a, b, c, d, x[ 5], KMD5_S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], KMD5_S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], KMD5_S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], KMD5_S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], KMD5_S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], KMD5_S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], KMD5_S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], KMD5_S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], KMD5_S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], KMD5_S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], KMD5_S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], KMD5_S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], KMD5_S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], KMD5_S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], KMD5_S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], KMD5_S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II (a, b, c, d, x[ 0], KMD5_S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], KMD5_S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], KMD5_S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], KMD5_S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], KMD5_S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], KMD5_S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], KMD5_S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], KMD5_S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], KMD5_S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], KMD5_S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], KMD5_S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], KMD5_S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], KMD5_S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], KMD5_S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], KMD5_S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], KMD5_S44, 0xeb86d391); /* 64 */

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;

    memset ( static_cast<void *>(x), 0, sizeof(x) );
}

inline quint32 KMD5::rotate_left (quint32 x, quint32 n)
{
    return (x << n) | (x >> (32-n))  ;
}

inline quint32 KMD5::F (quint32 x, quint32 y, quint32 z)
{
    return (x & y) | (~x & z);
}

inline quint32 KMD5::G (quint32 x, quint32 y, quint32 z)
{
    return (x & z) | (y & ~z);
}

inline quint32 KMD5::H (quint32 x, quint32 y, quint32 z)
{
    return x ^ y ^ z;
}

inline quint32 KMD5::I (quint32 x, quint32 y, quint32 z)
{
    return y ^ (x | ~z);
}

void KMD5::FF ( quint32& a, quint32 b, quint32 c, quint32 d,
                       quint32 x, quint32  s, quint32 ac )
{
    a += F(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::GG ( quint32& a, quint32 b, quint32 c, quint32 d,
                 quint32 x, quint32 s, quint32 ac)
{
    a += G(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::HH ( quint32& a, quint32 b, quint32 c, quint32 d,
                 quint32 x, quint32 s, quint32 ac )
{
    a += H(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::II ( quint32& a, quint32 b, quint32 c, quint32 d,
                 quint32 x, quint32 s, quint32 ac )
{
    a += I(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}


void KMD5::encode ( unsigned char* output, quint32 *in, quint32 len )
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    memcpy(output, in, len);
#else
    quint32 i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j]   = static_cast<quint8>((in[i] & 0xff));
        output[j+1] = static_cast<quint8>(((in[i] >> 8) & 0xff));
        output[j+2] = static_cast<quint8>(((in[i] >> 16) & 0xff));
        output[j+3] = static_cast<quint8>(((in[i] >> 24) & 0xff));
    }
#endif
}

// Decodes in (quint8) into output (quint32). Assumes len is a
// multiple of 4.
void KMD5::decode (quint32 *output, const unsigned char* in, quint32 len)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    memcpy(output, in, len);

#else
    quint32 i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = static_cast<quint32>(in[j]) |
                    (static_cast<quint32>(in[j+1]) << 8)  |
                    (static_cast<quint32>(in[j+2]) << 16) |
                    (static_cast<quint32>(in[j+3]) << 24);
#endif
}



/**************************************************************/
