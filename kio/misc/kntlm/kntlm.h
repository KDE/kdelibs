/*
   This file is part of the KDE libraries.
   Copyright (c) 2004 Szombathelyi Gy�rgy <gyurco@freemail.hu>
  
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef KNTLM_H
#define KNTLM_H

#include <qglobal.h>
#include <qcstring.h>
#include <qstring.h>

#include <kdelibs_export.h>

/**
 * @short KNTLM class implements the NTLM authentication protocol.
 *
 * The KNTLM class is useful for creating the authentication structures which 
 * can be used for various servers which implements NTLM type authentication.
 * A comprehensive description of the NTLM authentication protocol can be found
 * at http://davenport.sourceforge.net/ntlm.html
 * The class also contains methods to create the LanManager and NT (MD4) hashes
 * of a password. 
 * This class doesn't maintain any state information, so all methods are static.
 */

class KIO_EXPORT KNTLM {
public:

  enum Flags {
    Negotiate_Unicode         = 0x00000001,
    Negotiate_OEM             = 0x00000002,
    Request_Target            = 0x00000004,
    Negotiate_Sign            = 0x00000010,
    Negotiate_Seal            = 0x00000020,
    Negotiate_Datagram_Style  = 0x00000040,
    Negotiate_LM_Key          = 0x00000080,
    Negotiate_Netware         = 0x00000100,
    Negotiate_NTLM            = 0x00000200,
    Negotiate_Domain_Supplied = 0x00001000,
    Negotiate_WS_Supplied     = 0x00002000,
    Negotiate_Local_Call      = 0x00004000,
    Negotiate_Always_Sign     = 0x00008000,
    Target_Type_Domain        = 0x00010000,
    Target_Type_Server        = 0x00020000,
    Target_Type_Share         = 0x00040000,
    Negotiate_NTLM2_Key       = 0x00080000,
    Request_Init_Response     = 0x00100000,
    Request_Accept_Response   = 0x00200000,
    Request_NonNT_Key         = 0x00400000,
    Negotiate_Target_Info     = 0x00800000,
    Negotiate_128             = 0x20000000,
    Negotiate_Key_Exchange    = 0x40000000,
    Negotiate_56              = 0x80000000
  };

  typedef struct
  {
    Q_UINT16 len;
    Q_UINT16 maxlen;
    Q_UINT32 offset;
  } SecBuf;

  /**
   * The NTLM Type 1 structure
   */
  typedef struct
  {
    char signature[8]; /* "NTLMSSP\0" */
    Q_UINT32 msgType; /* 1 */
    Q_UINT32 flags;
    SecBuf domain;
    SecBuf workstation;
  } Negotiate;

  /**
   * The NTLM Type 2 structure
   */
  typedef struct
  {
    char signature[8];
    Q_UINT32 msgType; /* 2 */
    SecBuf targetName;
    Q_UINT32 flags;
    Q_UINT8 challengeData[8];
    Q_UINT32 context[2];
    SecBuf targetInfo;
  } Challenge;

  /**
   * The NTLM Type 3 structure
   */
  typedef struct
  {
    char signature[8];
    Q_UINT32 msgType; /* 3 */
    SecBuf lmResponse;
    SecBuf ntResponse;
    SecBuf domain;
    SecBuf user;
    SecBuf workstation;
    SecBuf sessionKey;
    Q_UINT32 flags;
  } Auth;
  
  typedef struct
  {
    Q_UINT32 signature;
    Q_UINT32 reserved;
    Q_UINT64 timestamp;
    Q_UINT8  challenge[8];
    Q_UINT8  unknown[4];
    //Target info block - variable length
  } Blob;

  /**
   * Creates the initial message (type 1) which should be sent to the server.
   *
   * @param negotiate - a buffer where the Type 1 message will returned.
   * @param domain - the domain name which should be send with the message.
   * @param workstation - the workstation name which should be send with the message.
   * @param flags - various flags, in most cases the defaults will good.
   *
   * @return true if creating the structure succeeds, false otherwise.   
   */
  static bool getNegotiate( QByteArray &negotiate, const QString &domain = QString::null, 
    const QString &workstation = QString::null,
    Q_UINT32 flags = Negotiate_Unicode | Request_Target | Negotiate_NTLM );
  /**
   * Creates the type 3 message which should be sent to the server after 
   * the challenge (type 2) received.
   *
   * @param auth - a buffer where the Type 3 message will returned.
   * @param challenge - the Type 2 message returned by the server.
   * @param user - user's name.
   * @param password - user's password.
   * @param domain - the target domain. If left empty, it will be extracted 
   * from the challenge.
   * @param workstation - the user's workstation.
   * @param forceNTLM - force the use of NTLM authentication (either v1 or v2).
   * @param forceNTLMv2 - force the use of NTLMv2 or LMv2 authentication. If false, NTLMv2 
   * support is autodetected from the challenge.
   *
   * @return true if auth filled with the Type 3 message, false if an error occured 
   * (challenge data invalid, or NTLM authentication forced, but the challenge data says
   * no NTLM supported).
   */
  static bool getAuth( QByteArray &auth, const QByteArray &challenge, const QString &user,
    const QString &password, const QString &domain = QString::null, 
    const QString &workstation = QString::null, bool forceNTLM = false, bool forceNTLMv2 = false );

  /**
   * Returns the LanManager response from the password and the server challenge.
   */
  static QByteArray getLMResponse( const QString &password, const unsigned char *challenge );
  /**
   * Calculates the LanManager hash of the specified password.
   */
  static QByteArray lmHash( const QString &password );
  /**
   * Calculates the LanManager response from the LanManager hash and the server challenge.
   */
  static QByteArray lmResponse( const QByteArray &hash, const unsigned char *challenge );

  /**
   * Returns the NTLM response from the password and the server challenge.
   */
  static QByteArray getNTLMResponse( const QString &password, const unsigned char *challenge );
  /**
   * Returns the NTLM hash (MD4) from the password.
   */
  static QByteArray ntlmHash( const QString &password );

  /**
   * Calculates the NTLMv2 response.
   */
  static QByteArray getNTLMv2Response( const QString &target, const QString &user, 
    const QString &password, const QByteArray &targetInformation, 
    const unsigned char *challenge );

  /**
   * Calculates the LMv2 response.
   */
  static QByteArray getLMv2Response( const QString &target, const QString &user,
    const QString &password, const unsigned char *challenge );

  /**
   * Returns the NTLMv2 hash.
   */
  static QByteArray ntlmv2Hash( const QString &target, const QString &user, const QString &password );

  /**
   * Calculates the LMv2 response.
   */
  static QByteArray lmv2Response( const QByteArray &hash,
    const QByteArray &clientData, const unsigned char *challenge );

  /**
   * Extracts a string field from an NTLM structure.
   */
  static QString getString( const QByteArray &buf, const SecBuf &secbuf, bool unicode );
  /**
   * Extracts a byte array from an NTLM structure.
   */
  static QByteArray getBuf( const QByteArray &buf, const SecBuf &secbuf );

  static QByteArray createBlob( const QByteArray &targetinfo );

  static QByteArray hmacMD5( const QByteArray &data, const QByteArray &key );
private:
  static QByteArray QString2UnicodeLE( const QString &target );
  static QString UnicodeLE2QString( const QChar* data, uint len );

  static void addBuf( QByteArray &buf, SecBuf &secbuf, QByteArray &data );
  static void addString( QByteArray &buf, SecBuf &secbuf, const QString &str, bool unicode = false );
  static void convertKey( unsigned char *key_56, void* ks );
};

#endif /* KNTLM_H */
