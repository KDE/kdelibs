/*
   Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   This KMD5 class is based on a C++ implementation of
   "RSA Data Security, Inc. MD5 Message-Digest Algorithm" by
   Mordechai T. Abzug,	Copyright (c) 1995.  This implementation
   passes the test-suite as defined by RFC 1321.

   RFC 1321 "MD5 Message-Digest Algorithm" Copyright (C) 1991-1992,
   RSA Data Security, Inc. Created 1991. All rights reserved.

   The encode/decode utilities in KCodecs were adapted from
   Ronald Tschalär Copyright (C) 1996-1999 HTTPClient java
   pacakge.
*/

#include <string.h>
#include <stdlib.h>

#include <kdebug.h>
#include "kmdcodec.h"

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

char KCodecs::Base64EncMap[64] = {
                                   0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
                                   0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
                                   0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
                                   0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
                                   0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
                                   0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
                                   0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33,
                                   0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F
                                 };

char KCodecs::Base64DecMap[128] = {
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
                                    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
                                    0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
                                    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                                    0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
                                    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                                    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
                                    0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
                                  };

char KCodecs::UUEncMap[64] = {
                               0x60, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                               0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                               0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                               0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                               0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                               0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
                               0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
                               0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F
                             };

char KCodecs::UUDecMap[128] = {
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                                0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              };

/******************************** KCodecs ********************************/
QCString KCodecs::base64Encode( const QCString& str )
{
    QByteArray in;
    in.resize( str.length() );
    memcpy( in.data(), str.data(), str.length() );
    return base64Encode( in );
}

QCString KCodecs::base64Encode( const QByteArray& in )
{
    QByteArray out;
    base64Encode( in, out );
    return QCString( out.data(), out.size()+1 );
}

void KCodecs::base64Encode( const QByteArray& in, QByteArray& out )
{
    if ( in.isEmpty() )
    {
        out.resize( 0 );
        return;
    }

    uint sidx=0;
    uint didx=0;
    uint len = in.size();

    out.resize( 0 );
    out.resize( ((len+2)/3)*4 );
    const Q_UINT8* buf = reinterpret_cast<Q_UINT8*>(in.data());

    // 3-byte to 4-byte conversion + 0-63 to ascii printable conversion
    for ( ; sidx < len-2; sidx += 3)
    {
        out[didx++] = Base64EncMap[(buf[sidx] >> 2) & 077];
        out[didx++] = Base64EncMap[(buf[sidx+1] >> 4) & 017 |
                                    (buf[sidx] << 4) & 077];
        out[didx++] = Base64EncMap[(buf[sidx+2] >> 6) & 003 |
                                   (buf[sidx+1] << 2) & 077];
        out[didx++] = Base64EncMap[buf[sidx+2] & 077];
    }

    if (sidx < len)
    {
        out[didx++] = Base64EncMap[(buf[sidx] >> 2) & 077];
        if (sidx < len-1)
        {
            out[didx++] = Base64EncMap[(buf[sidx+1] >> 4) & 017 |
                                       (buf[sidx] << 4) & 077];
            out[didx++] = Base64EncMap[(buf[sidx+1] << 2) & 077];
        }
        else
            out[didx++] = Base64EncMap[(buf[sidx] << 4) & 077];
    }

    // Add padding
    for ( ; didx < out.size(); didx++)
        out[didx] = '=';
}

QCString KBase64::base64Decode( const QCString& str )
{
    QByteArray in;
    in.resize( str.length() );
    memcpy( in.data(), str.data(), str.length() );
    return base64Decode( in );
}

QCString KBase64::base64Decode( const QByteArray& in )
{
    QByteArray out;
    base64Decode( in, out );
    return QCString( out.data(), out.size()+1 );
}

void KBase64::base64Decode( const QByteArray& in, QByteArray& out )
{
    if ( in.isEmpty() )
    {
        out.resize( 0 );
        return;
    }

    uint len = in.size();
    uint tail = len;

    while( in[tail-1] == '=' ) tail--;
    out.resize( 0 );
    out.resize( tail-(len/4) );
    Q_UINT8* buf = reinterpret_cast<Q_UINT8*>(in.data());

    // ascii printable to 0-63 conversion
    for (uint idx = 0; idx < len; idx++)
        buf[idx] = Base64DecMap[buf[idx]];

    // 4-byte to 3-byte conversion
    uint sidx=0, didx=0;
    for ( ; didx < out.size()-2; sidx += 4, didx += 3)
    {
        out[didx] = (((buf[sidx] << 2) & 255) | ((buf[sidx+1] >> 4) & 003));
        out[didx+1] = (((buf[sidx+1] << 4) & 255) | ((buf[sidx+2] >> 2) & 017));
        out[didx+2] = (((buf[sidx+2] << 6) & 255) | (buf[sidx+3] & 077) );
    }
    if (didx < out.size())
        out[didx] = (((buf[sidx] << 2) & 255) | ((buf[sidx+1] >> 4) & 003));

    if (++didx < out.size() )
        out[didx] = (((buf[sidx+1] << 4) & 255) | ((buf[sidx+2] >> 2) & 017));
}

QCString KCodecs::uuencode( const QCString& str )
{
    QByteArray in;
    in.resize( str.length() );
    memcpy( in.data(), str.data(), str.length() );
    return uuencode( in );
}

QCString KCodecs::uuencode( const QByteArray& in )
{
    QByteArray out;
    uuencode( in, out );
    return QCString( out.data(), out.size()+1 );
}

void KCodecs::uuencode( const QByteArray& in, QByteArray& out )
{
    if( in.isEmpty() )
    {
        out.resize( 0 );
        return;
    }

    uint sidx = 0;
    uint didx = 0;
    char nl[] = "\n";
    uint len = in.size();
    uint line_len = 45;
    uint nl_len = strlen(nl);
    out.resize( 0 );
    out.resize( (len+2)/3*4 + ((len+line_len-1)/line_len)*(nl_len+1) );
    const Q_UINT8* buf = reinterpret_cast<Q_UINT8*>(in.data());

    // split into lines, adding line-length and line terminator
    for ( ; sidx+line_len < len; )
    {
        // line length
        out[didx++] = UUEncMap[line_len];

        // 3-byte to 4-byte conversion + 0-63 to ascii printable conversion
        for (uint end = sidx+line_len; sidx < end; sidx += 3)
        {
            out[didx++] = UUEncMap[(buf[sidx] >> 2) & 077];
            out[didx++] = UUEncMap[(buf[sidx+1] >> 4) & 017 |
                                   (buf[sidx] << 4) & 077];
            out[didx++] = UUEncMap[(buf[sidx+2] >> 6) & 003 |
                                (buf[sidx+1] << 2) & 077];
            out[didx++] = UUEncMap[buf[sidx+2] & 077];
        }

        // line terminator
        for (uint idx=0; idx < nl_len; idx++)
            out[didx++] = nl[idx];
    }
    // line length
    out[didx++] = UUEncMap[len-sidx];
    // 3-byte to 4-byte conversion + 0-63 to ascii printable conversion
    for (; sidx+2 < len; sidx += 3)
    {
        out[didx++] = UUEncMap[(buf[sidx] >> 2) & 077];
        out[didx++] = UUEncMap[(buf[sidx+1] >> 4) & 017 |
                               (buf[sidx] << 4) & 077];
        out[didx++] = UUEncMap[(buf[sidx+2] >> 6) & 003 |
                               (buf[sidx+1] << 2) & 077];
        out[didx++] = UUEncMap[buf[sidx+2] & 077];
    }

    if (sidx < len-1)
    {
        out[didx++] = UUEncMap[(buf[sidx] >> 2) & 077];
        out[didx++] = UUEncMap[(buf[sidx+1] >> 4) & 017 |
                               (buf[sidx] << 4) & 077];
        out[didx++] = UUEncMap[(buf[sidx+1] << 2) & 077];
        out[didx++] = UUEncMap[0];
    }
    else if (sidx < len)
    {
        out[didx++] = UUEncMap[(buf[sidx] >> 2) & 077];
        out[didx++] = UUEncMap[(buf[sidx] << 4) & 077];
        out[didx++] = UUEncMap[0];
        out[didx++] = UUEncMap[0];
    }

    // line terminator
    for (uint idx=0; idx<nl_len; idx++)
        out[didx++] = nl[idx];

    // sanity check
    if ( didx !=  out.size()  )
        out.resize( 0 );
}

QCString KCodecs::uudecode( const QCString& str )
{
    QByteArray in;
    in.resize( str.length() );
    memcpy( in.data(), str.data(), str.length() );
    return uudecode( in );
}

QCString KCodecs::uudecode( const QByteArray& in )
{
    QByteArray out;
    uudecode( in, out );
    return QCString( out.data(), out.size()+1 );
}

void KCodecs::uudecode( const QByteArray& in, QByteArray& out )
{
    if( in.isEmpty() )
    {
        out.resize( 0 );
        return;
    }

    uint sidx = 0;
    uint didx = 0;
    uint len = in.size();
    uint line_len, end;
    out.resize( 0 );
    out.resize( len/4*3 );
    Q_UINT8* buf = reinterpret_cast<Q_UINT8*>(in.data());
    for (; sidx < len; )
    {
        // get line length (in number of encoded octets)
        line_len = UUDecMap[buf[sidx++]];
        // ascii printable to 0-63 and 4-byte to 3-byte conversion
        end = didx+line_len;
        for (; didx < end-2; sidx += 4)
        {
            char A = UUDecMap[buf[sidx]];
            char B = UUDecMap[buf[sidx+1]];
            char C = UUDecMap[buf[sidx+2]];
            char D = UUDecMap[buf[sidx+3]];
            out[didx++] = ( ((A << 2) & 255) | ((B >> 4) & 003) );
            out[didx++] = ( ((B << 4) & 255) | ((C >> 2) & 017) );
            out[didx++] = ( ((C << 6) & 255) | (D & 077) );
        }

        if (didx < end)
        {
            char A = UUDecMap[buf[sidx]];
            char B = UUDecMap[buf[sidx+1]];
            out[didx++] = ( ((A << 2) & 255) | ((B >> 4) & 003) );
        }

        if (didx < end)
        {
            char B = UUDecMap[buf[sidx+1]];
            char C = UUDecMap[buf[sidx+2]];
            out[didx++] = ( ((B << 4) & 255) | ((C >> 2) & 017) );
        }

        // skip padding
        while (sidx < len  && buf[sidx] != '\n' && buf[sidx] != '\r')
            sidx++;

        // skip end of line
        while (sidx < len  && (buf[sidx] == '\n' || buf[sidx] == '\r'))
            sidx++;
    }

    if ( didx > out.size()  )
    {
        QByteArray tmp ( out );
        tmp.detach();
        out.resize( 0 );
        out.resize( didx );
        memcpy( out.data(), tmp.data(), tmp.size() );
    }
}

/******************************** KMD5 ********************************/


KMD5::KMD5()
{
    init();
}

KMD5::KMD5( Q_UINT8 *in )
{
    init();
    update( in, qstrlen(reinterpret_cast<char *>(in)) );
    finalize();
}

KMD5::KMD5( const QCString& in )
{
    init();
    update( in );
    finalize();
}

KMD5::KMD5( const QString& in )
{
    init();
    update( in );
    finalize();
}

KMD5::KMD5( const QByteArray& in )
{
    init();
    update( in );
    finalize();
}

KMD5::KMD5(FILE *f)
{
    init();
    update( f, true );
    finalize ();
}

void KMD5::update ( const QString& in )
{
    update( QCString( in.latin1() ) );
}

void KMD5::update ( const QCString& in )
{
    update( reinterpret_cast<Q_UINT8*>(in.data()), in.length() );
}

void KMD5::update( const QByteArray& in )
{
    update( reinterpret_cast<Q_UINT8*>(in.data()), in.size() );
}

void KMD5::update( Q_UINT8 *in, int len )
{
    if ( len == -1 )
        len = qstrlen( reinterpret_cast<char*>(in) );

    Q_UINT32 in_index;
    Q_UINT32 buffer_index;
    Q_UINT32 buffer_space;
    Q_UINT32 in_length = static_cast<Q_UINT32>( len );

    if (m_finalized)
    {
        m_error = ERR_ALREADY_FINALIZED;
        return;
    }

    buffer_index = static_cast<Q_UINT32>((m_count[0] >> 3) & 0x3F);

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
            transform (in+in_index);

        buffer_index = 0;
    }
    else
        in_index=0;

    memcpy(m_buffer+buffer_index, in+in_index, in_length-in_index);
}

void KMD5::update( FILE *file, bool closeFile )
{
    Q_UINT8 buffer[1024];
    int len;

    while ((len=fread(buffer, 1, 1024, file)))
        update(buffer, len);

    // Check if we got to this point because
    // we reached EOF or an error.
    if ( !feof( file ) )
    {
        m_error = ERR_CANNOT_READ_FILE;
        return;
    }

    // Close the file iff the flag is set.
    if ( closeFile && fclose (file) )
        m_error = ERR_CANNOT_CLOSE_FILE;
}

void KMD5::finalize ()
{

    Q_UINT8 bits[8];
    Q_UINT32 index, padLen;
    static Q_UINT8 PADDING[64]=
    {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if (m_finalized)
    {
        m_error = ERR_ALREADY_FINALIZED;
        return;
    }

    encode (bits, m_count, 8);

    // Pad out to 56 mod 64.
    index = static_cast<Q_UINT32>((m_count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    update (PADDING, padLen);

    // Append length (before padding)
    update (bits, 8);

    // Store state in digest
    encode (m_digest, m_state, 16);

    // Fill sensitive information with zero's
    memset ( (void *)m_buffer, 0, sizeof(*m_buffer));

    m_finalized = true;
}

void KMD5::reset()
{
    init();
}

bool KMD5::verify( const char * msg_digest, DigestType type )
{
    if ( !m_finalized || !m_digest || m_error!=ERR_NONE )
      return false;
    return isDigestMatch( msg_digest,  type );
}

bool KMD5::verify( FILE* f, const char * msg_digest, DigestType type )
{
    init();
    update( f );
    finalize();
    return isDigestMatch( msg_digest,  type );
}

bool KMD5::verify( const QCString& in, const char * msg_digest,
                   DigestType type )
{
    init();
    update( in );
    finalize();
    return isDigestMatch( msg_digest,  type );
}

bool KMD5::verify( const QString& in, const char * msg_digest,
                   DigestType type )
{
  return verify( QCString(in.latin1()), msg_digest, type );
}

Q_UINT8* KMD5::rawDigest()
{
    Q_UINT8* s = new Q_UINT8[16];
    rawDigest( (char*)s );
    if ( m_error == ERR_NONE )
        return s;
    else
        return '\0';
}

void KMD5::rawDigest( HASH bin )
{
    if (!m_finalized)
    {
        m_error = ERR_NOT_YET_FINALIZED;
        return;
    }
    memcpy( bin, m_digest, 16 );
}

char * KMD5::hexDigest()
{
    char *s= new char[33];
    hexDigest( s );
    if ( m_error == ERR_NONE )
        return s;
    else
        return 0;
}

void KMD5::hexDigest( HASHHEX hex )
{
    if (!m_finalized)
    {
        m_error = ERR_NOT_YET_FINALIZED;
        return;
    }

    for (int i=0; i<16; i++)
        sprintf(hex+i*2, "%02x", m_digest[i]);
    hex[32]='\0';
}

void KMD5::init()
{
    m_finalized= false;
    m_error = ERR_NONE;

    m_count[0] = 0;
    m_count[1] = 0;

    m_state[0] = 0x67452301;
    m_state[1] = 0xefcdab89;
    m_state[2] = 0x98badcfe;
    m_state[3] = 0x10325476;

    memset ( (void *) m_buffer, 0, sizeof(*m_buffer));
    memset ( (void *) m_digest, 0, sizeof(*m_digest));
}

bool KMD5::isDigestMatch( const char * msg_digest, DigestType type )
{
    bool result = false;

    switch (type)
    {
        case HEX:
            if ( strcmp( hexDigest(), msg_digest ) == 0 )
                result = true;
            break;
        case BIN:
            if ( strcmp( reinterpret_cast<char *>(rawDigest()), msg_digest ) == 0 )
                result = true;
            break;
        default:
            break;
    }
    return result;
}

void KMD5::transform( Q_UINT8 block[64] )
{

    Q_UINT32 a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3], x[16];

    decode (x, block, 64);
    ASSERT(!m_finalized);  // not just a user error, since the method is private

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

    memset ( (void *) x, 0, sizeof(x) );
}

inline Q_UINT32 KMD5::rotate_left (Q_UINT32 x, Q_UINT32 n)
{
    return (x << n) | (x >> (32-n))  ;
}

inline Q_UINT32 KMD5::F (Q_UINT32 x, Q_UINT32 y, Q_UINT32 z)
{
    return (x & y) | (~x & z);
}

inline Q_UINT32 KMD5::G (Q_UINT32 x, Q_UINT32 y, Q_UINT32 z)
{
    return (x & z) | (y & ~z);
}

inline Q_UINT32 KMD5::H (Q_UINT32 x, Q_UINT32 y, Q_UINT32 z)
{
    return x ^ y ^ z;
}

inline Q_UINT32 KMD5::I (Q_UINT32 x, Q_UINT32 y, Q_UINT32 z)
{
    return y ^ (x | ~z);
}

void KMD5::FF ( Q_UINT32& a, Q_UINT32 b, Q_UINT32 c, Q_UINT32 d,
                       Q_UINT32 x, Q_UINT32  s, Q_UINT32 ac )
{
    a += F(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::GG ( Q_UINT32& a, Q_UINT32 b, Q_UINT32 c, Q_UINT32 d,
                 Q_UINT32 x, Q_UINT32 s, Q_UINT32 ac)
{
    a += G(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::HH ( Q_UINT32& a, Q_UINT32 b, Q_UINT32 c, Q_UINT32 d,
                 Q_UINT32 x, Q_UINT32 s, Q_UINT32 ac )
{
    a += H(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::II ( Q_UINT32& a, Q_UINT32 b, Q_UINT32 c, Q_UINT32 d,
                 Q_UINT32 x, Q_UINT32 s, Q_UINT32 ac )
{
    a += I(b, c, d) + x + ac;
    a = rotate_left (a, s) +b;
}

void KMD5::encode ( Q_UINT8 *output, Q_UINT32 *in, Q_UINT32 len )
{
    Q_UINT32 i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j]   = static_cast<Q_UINT8>((in[i] & 0xff));
        output[j+1] = static_cast<Q_UINT8>(((in[i] >> 8) & 0xff));
        output[j+2] = static_cast<Q_UINT8>(((in[i] >> 16) & 0xff));
        output[j+3] = static_cast<Q_UINT8>(((in[i] >> 24) & 0xff));
    }
}

// Decodes in (Q_UINT8) into output (Q_UINT32). Assumes len is a
// multiple of 4.
void KMD5::decode (Q_UINT32 *output, Q_UINT8 *in, Q_UINT32 len)
{
    Q_UINT32 i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = static_cast<Q_UINT32>(in[j]) |
                    (static_cast<Q_UINT32>(in[j+1]) << 8)  |
                    (static_cast<Q_UINT32>(in[j+2]) << 16) |
                    (static_cast<Q_UINT32>(in[j+3]) << 24);
}

/**** Functions provided for backwards compatibility ****/

QString KCodecs::base64Encode( const QString& str ){
    return base64Encode(QCString(str.latin1())); }

QString KCodecs::base64Decode( const QString& str ) {
    return base64Decode(QCString(str.latin1())); }

QString KCodecs::uuencode( const QString& str )  {
    return uuencode(QCString(str.latin1())); }

QString KCodecs::uudecode( const QString& str )  {
    return uudecode(QCString(str.latin1())); }

QString KCodecs::encodeString( const QString& data ) {
    return base64Encode(data); }

QString KCodecs::decodeString( const QString& data ) {
    return base64Decode(data); }

