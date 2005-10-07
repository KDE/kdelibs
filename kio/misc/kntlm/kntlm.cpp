/* This file is part of the KDE libraries
   Copyright (c) 2004 Szombathelyi Gy�gy <gyurco@freemail.hu>

   The implementation is based on the documentation and sample code
   at http://davenport.sourceforge.net/ntlm.html
   The DES encryption functions are from libntlm 
   at http://josefsson.org/libntlm/

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <string.h>

#include <qdatetime.h>
#include <krandom.h>
#include <kswap.h>
#include <kcodecs.h>
#include <kdebug.h>

#include "des.h"
#include "kntlm.h"

QString KNTLM::getString( const QByteArray &buf, const SecBuf &secbuf, bool unicode )
{
  //watch for buffer overflows
  if ( secbuf.offset > buf.size() ||
       secbuf.offset + secbuf.len > buf.size() ) return QString::null;

  QString str;
  const char *c = buf.data() + secbuf.offset;
  
  if ( unicode ) {
    str = UnicodeLE2QString( (QChar*) c, secbuf.len >> 1 );
  } else {
    str = QString::fromLatin1( c, secbuf.len );
  }
  return str;
}

QByteArray KNTLM::getBuf( const QByteArray &buf, const SecBuf &secbuf )
{
  QByteArray ret;
  //watch for buffer overflows
  if ( secbuf.offset > buf.size() ||
       secbuf.offset + secbuf.len > buf.size() ) return ret;
  ret.duplicate( buf.data() + secbuf.offset, buf.size() );
  return ret;
}

void KNTLM::addString( QByteArray &buf, SecBuf &secbuf, const QString &str, bool unicode )
{
  QByteArray tmp;

  if ( unicode ) {
    tmp = QString2UnicodeLE( str );
    addBuf( buf, secbuf, tmp );
  } else {
    const char *c;
    c = str.latin1();
    tmp.setRawData( c, str.length() );
    addBuf( buf, secbuf, tmp );
    tmp.resetRawData( c, str.length() );
  }
}

void KNTLM::addBuf( QByteArray &buf, SecBuf &secbuf, QByteArray &data )
{
  secbuf.offset = (buf.size() + 1) & 0xfffffffe;
  secbuf.len = data.size();
  secbuf.maxlen = data.size();
  buf.resize( secbuf.offset + data.size() );
  memcpy( buf.data() + secbuf.offset, data.data(), data.size() );
}

bool KNTLM::getNegotiate( QByteArray &negotiate, const QString &domain, const QString &workstation, quint32 flags )
{
  QByteArray rbuf( sizeof(Negotiate) );
  
  rbuf.fill( 0 );
  memcpy( rbuf.data(), "NTLMSSP", 8 );
  ((Negotiate*) rbuf.data())->msgType = KFromToLittleEndian( (quint32)1 );
  if ( !domain.isEmpty() ) {
    flags |= Negotiate_Domain_Supplied;
    addString( rbuf, ((Negotiate*) rbuf.data())->domain, domain );
  }
  if ( !workstation.isEmpty() ) {
    flags |= Negotiate_WS_Supplied;
    addString( rbuf, ((Negotiate*) rbuf.data())->domain, workstation );
  }
  ((Negotiate*) rbuf.data())->flags = KFromToLittleEndian( flags );
  negotiate = rbuf;
  return true;
}

bool KNTLM::getAuth( QByteArray &auth, const QByteArray &challenge, const QString &user, 
  const QString &password, const QString &domain, const QString &workstation, 
  bool forceNTLM, bool forceNTLMv2 )
{
  QByteArray rbuf( sizeof(Auth) );
  Challenge *ch = (Challenge *) challenge.data();
  QByteArray response;
  uint chsize = challenge.size();
  bool unicode = false;
  QString dom;

  //challenge structure too small
  if ( chsize < 32 ) return false;

  unicode = ch->flags & Negotiate_Unicode;
  if ( domain.isEmpty() )
    dom = getString( challenge, ch->targetName, unicode );
  else
    dom = domain;
    
  rbuf.fill( 0 );
  memcpy( rbuf.data(), "NTLMSSP", 8 );
  ((Auth*) rbuf.data())->msgType = KFromToLittleEndian( (quint32)3 );
  ((Auth*) rbuf.data())->flags = ch->flags;
  QByteArray targetInfo = getBuf( challenge, ch->targetInfo );

  if ( forceNTLMv2 || (!targetInfo.isEmpty() && (ch->flags & Negotiate_Target_Info)) /* may support NTLMv2 */ ) {
    if ( ch->flags & Negotiate_NTLM ) {
      if ( targetInfo.isEmpty() ) return false;
      response = getNTLMv2Response( dom, user, password, targetInfo, ch->challengeData );
      addBuf( rbuf, ((Auth*) rbuf.data())->ntResponse, response );
    } else {
      if ( !forceNTLM ) {
        response = getLMv2Response( dom, user, password, ch->challengeData );
        addBuf( rbuf, ((Auth*) rbuf.data())->lmResponse, response );
      } else 
        return false;
    }
  } else { //if no targetinfo structure and NTLMv2 or LMv2 not forced, try the older methods
    if ( ch->flags & Negotiate_NTLM ) {
      response = getNTLMResponse( password, ch->challengeData );
      addBuf( rbuf, ((Auth*) rbuf.data())->ntResponse, response );
    } else {
      if ( !forceNTLM ) {
        response = getLMResponse( password, ch->challengeData );
        addBuf( rbuf, ((Auth*) rbuf.data())->lmResponse, response );
      } else
        return false;
    }
  }
  if ( !dom.isEmpty() )
    addString( rbuf, ((Auth*) rbuf.data())->domain, dom, unicode );
  addString( rbuf, ((Auth*) rbuf.data())->user, user, unicode );
  if ( !workstation.isEmpty() )
    addString( rbuf, ((Auth*) rbuf.data())->workstation, workstation, unicode );

  auth = rbuf;

  return true;
}

QByteArray KNTLM::getLMResponse( const QString &password, const unsigned char *challenge )
{
  QByteArray hash, answer;

  hash = lmHash( password );
  hash.resize( 21 );
  memset( hash.data() + 16, 0, 5 );
  answer = lmResponse( hash, challenge );
  hash.fill( 0 );
  return answer;
}

QByteArray KNTLM::lmHash( const QString &password )
{
  QByteArray keyBytes( 14 );
  QByteArray hash( 16 );
  DES_KEY ks;
  const char *magic = "KGS!@#$%";

  keyBytes.fill( 0 );
  strncpy( keyBytes.data(), password.toUpper().latin1(), 14 );

  convertKey( (unsigned char*) keyBytes.data(), &ks );
  ntlm_des_ecb_encrypt( magic, 8, &ks, (unsigned char*) hash.data() );

  convertKey( (unsigned char*) keyBytes.data() + 7, &ks );
  ntlm_des_ecb_encrypt( magic, 8, &ks, (unsigned char*) hash.data() + 8 );

  keyBytes.fill( 0 );
  memset( &ks, 0, sizeof (ks) );

  return hash;
}

QByteArray KNTLM::lmResponse( const QByteArray &hash, const unsigned char *challenge )
{
  DES_KEY ks;
  QByteArray answer( 24 );

  convertKey( (unsigned char*) hash.data(), &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() );

  convertKey( (unsigned char*) hash.data() + 7, &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() + 8 );

  convertKey( (unsigned char*) hash.data() + 14, &ks );
  ntlm_des_ecb_encrypt( challenge, 8, &ks, (unsigned char*) answer.data() + 16 );

  memset( &ks, 0, sizeof (ks) );
  return answer;
}

QByteArray KNTLM::getNTLMResponse( const QString &password, const unsigned char *challenge )
{
  QByteArray hash, answer;

  hash = ntlmHash( password );
  hash.resize( 21 );
  memset( hash.data() + 16, 0, 5 );
  answer = lmResponse( hash, challenge );
  hash.fill( 0 );
  return answer;
}

QByteArray KNTLM::ntlmHash( const QString &password )
{
  KMD4::Digest digest;
  QByteArray ret, unicode;
  unicode = QString2UnicodeLE( password );

  KMD4 md4( unicode );
  md4.rawDigest( digest );
  ret.duplicate( (const char*) digest, sizeof( digest ) );
  return ret;
}

QByteArray KNTLM::getNTLMv2Response( const QString &target, const QString &user,
  const QString &password, const QByteArray &targetInformation,
  const unsigned char *challenge )
{
  QByteArray hash = ntlmv2Hash( target, user, password );
  QByteArray blob = createBlob( targetInformation );
  return lmv2Response( hash, blob, challenge );
}

QByteArray KNTLM::getLMv2Response( const QString &target, const QString &user,
  const QString &password, const unsigned char *challenge )
{
  QByteArray hash = ntlmv2Hash( target, user, password );
  QByteArray clientChallenge( 8 );
  for ( uint i = 0; i<8; i++ ) {
    clientChallenge.data()[i] = KRandom::random() % 0xff;
  }
  return lmv2Response( hash, clientChallenge, challenge );
}

QByteArray KNTLM::ntlmv2Hash( const QString &target, const QString &user, const QString &password )
{
  QByteArray hash1 = ntlmHash( password );
  QByteArray key, ret;
  QString id = user.toUpper() + target.toUpper();
  key = QString2UnicodeLE( id );
  ret = hmacMD5( key, hash1 );
  return ret;  
}

QByteArray KNTLM::lmv2Response( const QByteArray &hash, 
  const QByteArray &clientData, const unsigned char *challenge )
{
  QByteArray data( 8 + clientData.size() );
  memcpy( data.data(), challenge, 8 );
  memcpy( data.data() + 8, clientData.data(), clientData.size() );
  QByteArray mac = hmacMD5( data, hash );
  mac.resize( 16 + clientData.size() );
  memcpy( mac.data() + 16, clientData.data(), clientData.size() );
  return mac;
}

QByteArray KNTLM::createBlob( const QByteArray &targetinfo )
{
  QByteArray blob( sizeof(Blob) + 4 + targetinfo.size() );
  blob.fill( 0 );
  
  Blob *bl = (Blob *) blob.data();
  bl->signature = KFromToBigEndian( (quint32) 0x01010000 );
  quint64 now = QDateTime::currentDateTime().toTime_t();
  now += (quint64)3600*(quint64)24*(quint64)134774;
  now *= (quint64)10000000;
  bl->timestamp = KFromToLittleEndian( now );
  for ( uint i = 0; i<8; i++ ) {
    bl->challenge[i] = KRandom::random() % 0xff;
  }
  memcpy( blob.data() + sizeof(Blob), targetinfo.data(), targetinfo.size() );
  return blob;
}

QByteArray KNTLM::hmacMD5( const QByteArray &data, const QByteArray &key )
{
  quint8 ipad[64], opad[64];
  KMD5::Digest digest;
  QByteArray ret;
  
  memset( ipad, 0x36, sizeof(ipad) );
  memset( opad, 0x5c, sizeof(opad) );
  for ( int i = key.size()-1; i >= 0; i-- ) {
    ipad[i] ^= key[i];
    opad[i] ^= key[i];
  }

  QByteArray content( data.size()+64 );
  memcpy( content.data(), ipad, 64 );
  memcpy( content.data() + 64, data.data(), data.size() );
  KMD5 md5( content );
  md5.rawDigest( digest );
  content.resize( sizeof(digest) + 64 );
  memcpy( content.data(), opad, 64 );
  memcpy( content.data() + 64, digest, sizeof(digest) );
  md5.reset();
  md5.update( content );
  md5.rawDigest( digest );

  ret.duplicate( (const char*) digest, sizeof( digest ) );
  return ret;
}

/*
* turns a 56 bit key into the 64 bit, odd parity key and sets the key.
* The key schedule ks is also set.
*/
void KNTLM::convertKey( unsigned char *key_56, void* ks )
{
  unsigned char key[8];

  key[0] = key_56[0];
  key[1] = ((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1);
  key[2] = ((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2);
  key[3] = ((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3);
  key[4] = ((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4);
  key[5] = ((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5);
  key[6] = ((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6);
  key[7] = (key_56[6] << 1) & 0xFF;

  for ( uint i=0; i<8; i++ ) {
    unsigned char b = key[i];
    bool needsParity = (((b>>7) ^ (b>>6) ^ (b>>5) ^ (b>>4) ^ (b>>3) ^ (b>>2) ^ (b>>1)) & 0x01) == 0;
    if ( needsParity ) 
      key[i] |= 0x01;
    else
      key[i] &= 0xfe;
  }

  ntlm_des_set_key ( (DES_KEY*) ks, (char*) &key, sizeof (key));

  memset (&key, 0, sizeof (key));
}

QByteArray KNTLM::QString2UnicodeLE( const QString &target )
{
  QByteArray unicode( target.length() * 2 );
  for ( int i = 0; i < target.length(); i++ ) {
    ((quint16*)unicode.data())[ i ] = KFromToLittleEndian( target[i].unicode() );
  }
  return unicode;
}

QString KNTLM::UnicodeLE2QString( const QChar* data, uint len )
{
  QString ret;
  for ( uint i = 0; i < len; i++ ) {
    ret += KFromToLittleEndian( data[ i ].unicode() );
  }
  return ret;
}
