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

#include "kntlm.h"
#include "des.h"

#include <cstring>

#include <QtCore/QDate>
#include <QtCore/QtEndian>
#include <QtCore/QCryptographicHash>

#include <krandom.h>
#include <kdebug.h>

static const char NTLM_SIGNATURE[] = "NTLMSSP";

static QByteArray QString2UnicodeLE (const QString &target)
{
    QByteArray unicode (target.length() * 2, 0);

    for (int i = 0; i < target.length(); i++) {
        ((quint16 *) unicode.data()) [ i ] = qToLittleEndian (target[i].unicode());
    }

    return unicode;
}

static QString UnicodeLE2QString (const QChar *data, uint len)
{
    QString ret;

    for (uint i = 0; i < len; i++) {
        ret += qFromLittleEndian (data[ i ].unicode());
    }

    return ret;
}

static QByteArray getBuf (const QByteArray &buf, const KNTLM::SecBuf &secbuf)
{
    quint32 offset = qFromLittleEndian ( (quint32) secbuf.offset);
    quint16 len = qFromLittleEndian (secbuf.len);

    //watch for buffer overflows
    if (offset > (quint32) buf.size() || offset + len > (quint32) buf.size()) {
        return QByteArray();
    }

    return QByteArray (buf.data() + offset, buf.size());
}

static void addBuf (QByteArray &buf, KNTLM::SecBuf &secbuf, const QByteArray &data)
{
    quint32 offset = (buf.size() + 1) & 0xfffffffe;
    quint16 len = data.size();
    quint16 maxlen = data.size();

    secbuf.offset = qToLittleEndian ( (quint32) offset);
    secbuf.len = qToLittleEndian (len);
    secbuf.maxlen = qToLittleEndian (maxlen);
    buf.resize (offset + len);
    memcpy (buf.data() + offset, data.data(), data.size());
}

static QString getString (const QByteArray &buf, const KNTLM::SecBuf &secbuf, bool unicode)
{
    //watch for buffer overflows
    quint32 offset = qFromLittleEndian ( (quint32) secbuf.offset);
    quint16 len = qFromLittleEndian (secbuf.len);

    if (offset > (quint32) buf.size() || offset + len > (quint32) buf.size()) {
        return QString();
    }

    const char *c = buf.data() + offset;

    if (unicode) {
        return UnicodeLE2QString ( (QChar *) c, len >> 1);
    }

    return QString::fromLatin1 (c, len);
}

static void addString (QByteArray &buf, KNTLM::SecBuf &secbuf, const QString &str, bool unicode = false)
{
    if (unicode) {
        addBuf (buf, secbuf, QString2UnicodeLE (str));
        return;
    }

    addBuf (buf, secbuf, str.toLatin1());
}

/*
* turns a 56 bit key into the 64 bit, odd parity key and sets the key.
* The key schedule ks is also set.
*/
static void convertKey (unsigned char *key_56, void *ks)
{
    unsigned char key[8];

    key[0] = key_56[0];
    key[1] = ( (key_56[0] << 7) & 0xFF) | (key_56[1] >> 1);
    key[2] = ( (key_56[1] << 6) & 0xFF) | (key_56[2] >> 2);
    key[3] = ( (key_56[2] << 5) & 0xFF) | (key_56[3] >> 3);
    key[4] = ( (key_56[3] << 4) & 0xFF) | (key_56[4] >> 4);
    key[5] = ( (key_56[4] << 3) & 0xFF) | (key_56[5] >> 5);
    key[6] = ( (key_56[5] << 2) & 0xFF) | (key_56[6] >> 6);
    key[7] = (key_56[6] << 1) & 0xFF;

    for (uint i = 0; i < 8; i++) {
        unsigned char b = key[i];
        bool needsParity = ((((b >> 7) ^ (b >> 6) ^ (b >> 5) ^ (b >> 4) ^ (b >> 3) ^ (b >> 2) ^ (b >> 1)) & 0x01) == 0);

        if (needsParity) {
            key[i] |= 0x01;
        } else {
            key[i] &= 0xfe;
        }
    }

    ntlm_des_set_key ( (DES_KEY *) ks, (char *) &key, sizeof (key));
    memset (&key, 0, sizeof (key));
}

static QByteArray createBlob (const QByteArray &targetinfo)
{
    QByteArray blob (sizeof (KNTLM::Blob) + 4 + targetinfo.size(), 0);

    KNTLM::Blob *bl = (KNTLM::Blob *) blob.data();
    bl->signature = qToBigEndian ( (quint32) 0x01010000);
    quint64 now = QDateTime::currentDateTime().toTime_t();
    now += (quint64) 3600 * (quint64) 24 * (quint64) 134774;
    now *= (quint64) 10000000;
    bl->timestamp = qToLittleEndian (now);

    for (uint i = 0; i < 8; i++) {
        bl->challenge[i] = KRandom::random() % 0xff;
    }

    memcpy (blob.data() + sizeof (KNTLM::Blob), targetinfo.data(), targetinfo.size());
    return blob;
}

static QByteArray hmacMD5 (const QByteArray &data, const QByteArray &key)
{
    quint8 ipad[64], opad[64];
    QByteArray ret;

    memset (ipad, 0x36, sizeof (ipad));
    memset (opad, 0x5c, sizeof (opad));

    for (int i = key.size() - 1; i >= 0; i--) {
        ipad[i] ^= key[i];
        opad[i] ^= key[i];
    }

    QByteArray content (data.size() + 64, 0);
    memcpy (content.data(), ipad, 64);
    memcpy (content.data() + 64, data.data(), data.size());

    QCryptographicHash md5 (QCryptographicHash::Md5);
    md5.addData (content);
    content.resize (64);
    memcpy (content.data(), opad, 64);
    content += md5.result();

    md5.reset();
    md5.addData (content);

    return md5.result();
}


/*************************************** KNTLM implementation ***************************************/

bool KNTLM::getNegotiate (QByteArray &negotiate, const QString &domain, const QString &workstation, quint32 flags)
{
    QByteArray rbuf (sizeof (Negotiate), 0);

    memcpy (rbuf.data(), NTLM_SIGNATURE, sizeof (NTLM_SIGNATURE));
    ((Negotiate *) rbuf.data())->msgType = qToLittleEndian ( (quint32) 1);

    if (!domain.isEmpty()) {
        flags |= Negotiate_Domain_Supplied;
        addString (rbuf, ((Negotiate *) rbuf.data())->domain, domain);
    }

    if (!workstation.isEmpty()) {
        flags |= Negotiate_WS_Supplied;
        addString (rbuf, ((Negotiate *) rbuf.data())->workstation, workstation);
    }

    ((Negotiate *) rbuf.data())->flags = qToLittleEndian (flags);
    negotiate = rbuf;
    return true;
}

bool KNTLM::getAuth (QByteArray &auth, const QByteArray &challenge,
                     const QString &user, const QString &password, const QString &domain,
                     const QString &workstation, AuthFlags authflags)
{
    QByteArray rbuf (sizeof (Auth), 0);
    Challenge *ch = (Challenge *) challenge.data();
    QByteArray response;
    uint chsize = challenge.size();
    bool unicode = false;
    QString dom;

    //challenge structure too small
    if (chsize < 32) {
        return false;
    }

    unicode = qFromLittleEndian (ch->flags) & Negotiate_Unicode;

    if (domain.isEmpty()) {
        dom = getString (challenge, ch->targetName, unicode);
    } else {
        dom = domain;
    }

    memcpy (rbuf.data(), NTLM_SIGNATURE, sizeof (NTLM_SIGNATURE));
    ((Auth *) rbuf.data())->msgType = qToLittleEndian ( (quint32) 3);
    ((Auth *) rbuf.data())->flags = ch->flags;
    QByteArray targetInfo = getBuf (challenge, ch->targetInfo);

    if ( ( (authflags & Force_V2) && ! (authflags & Force_V1)) ||
            (!targetInfo.isEmpty() && (qFromLittleEndian (ch->flags) & Negotiate_Target_Info)) /* may support NTLMv2 */) {
        bool ret = false;

        if (qFromLittleEndian (ch->flags) & Negotiate_NTLM) {
            if (targetInfo.isEmpty())
                return false;

            response = getNTLMv2Response (dom, user, password, targetInfo, ch->challengeData);
            addBuf (rbuf, ((Auth *) rbuf.data())->ntResponse, response);
            ret = true;
        }

        if (authflags & Add_LM) {
            response = getLMv2Response (dom, user, password, ch->challengeData);
            addBuf (rbuf, ((Auth *) rbuf.data())->lmResponse, response);
            ret = true;
        }

        if (!ret) {
            return false;
        }
    } else { //if no targetinfo structure and NTLMv2 or LMv2 not forced, or v1 forced, try the older methods
        bool ret = false;

        if (qFromLittleEndian (ch->flags) & Negotiate_NTLM) {
            response = getNTLMResponse (password, ch->challengeData);
            addBuf (rbuf, ((Auth *) rbuf.data())->ntResponse, response);
            ret = true;
        }

        if (authflags & Add_LM) {
            response = getLMResponse (password, ch->challengeData);
            addBuf (rbuf, ((Auth *) rbuf.data())->lmResponse, response);
            ret = true;
        }

        if (!ret) {
            return false;
        }
    }

    if (!dom.isEmpty()) {
        addString (rbuf, ((Auth *) rbuf.data())->domain, dom, unicode);
    }

    addString (rbuf, ((Auth *) rbuf.data())->user, user, unicode);

    if (!workstation.isEmpty()) {
        addString (rbuf, ((Auth *) rbuf.data())->workstation, workstation, unicode);
    }

    auth = rbuf;
    return true;
}

QByteArray KNTLM::getLMResponse (const QString &password, const unsigned char *challenge)
{
    QByteArray hash, answer;

    hash = lmHash (password);
    hash.resize (21);
    memset (hash.data() + 16, 0, 5);
    answer = lmResponse (hash, challenge);
    hash.fill (0);
    return answer;
}

QByteArray KNTLM::lmHash (const QString &password)
{
    QByteArray keyBytes (14, 0);
    QByteArray hash (16, 0);
    DES_KEY ks;
    const char *magic = "KGS!@#$%";

    strncpy (keyBytes.data(), password.toUpper().toLatin1(), 14);

    convertKey ( (unsigned char *) keyBytes.data(), &ks);
    ntlm_des_ecb_encrypt (magic, 8, &ks, (unsigned char *) hash.data());

    convertKey ( (unsigned char *) keyBytes.data() + 7, &ks);
    ntlm_des_ecb_encrypt (magic, 8, &ks, (unsigned char *) hash.data() + 8);

    keyBytes.fill (0);
    memset (&ks, 0, sizeof (ks));

    return hash;
}

QByteArray KNTLM::lmResponse (const QByteArray &hash, const unsigned char *challenge)
{
    DES_KEY ks;
    QByteArray answer (24, 0);

    convertKey ( (unsigned char *) hash.data(), &ks);
    ntlm_des_ecb_encrypt (challenge, 8, &ks, (unsigned char *) answer.data());

    convertKey ( (unsigned char *) hash.data() + 7, &ks);
    ntlm_des_ecb_encrypt (challenge, 8, &ks, (unsigned char *) answer.data() + 8);

    convertKey ( (unsigned char *) hash.data() + 14, &ks);
    ntlm_des_ecb_encrypt (challenge, 8, &ks, (unsigned char *) answer.data() + 16);

    memset (&ks, 0, sizeof (ks));
    return answer;
}

QByteArray KNTLM::getNTLMResponse (const QString &password, const unsigned char *challenge)
{
    QByteArray hash = ntlmHash (password);
    hash.resize (21);
    memset (hash.data() + 16, 0, 5);
    QByteArray answer = lmResponse (hash, challenge);
    hash.fill (0);
    return answer;
}

QByteArray KNTLM::ntlmHash (const QString &password)
{
    QByteArray unicode;
    unicode = QString2UnicodeLE (password);

    return QCryptographicHash::hash (unicode, QCryptographicHash::Md4);
}

QByteArray KNTLM::getNTLMv2Response (const QString &target, const QString &user,
                                     const QString &password, const QByteArray &targetInformation,
                                     const unsigned char *challenge)
{
    QByteArray hash = ntlmv2Hash (target, user, password);
    QByteArray blob = createBlob (targetInformation);
    return lmv2Response (hash, blob, challenge);
}

QByteArray KNTLM::getLMv2Response (const QString &target, const QString &user,
                                   const QString &password, const unsigned char *challenge)
{
    QByteArray hash = ntlmv2Hash (target, user, password);
    QByteArray clientChallenge (8, 0);

    for (uint i = 0; i < 8; i++) {
        clientChallenge.data() [i] = KRandom::random() % 0xff;
    }

    return lmv2Response (hash, clientChallenge, challenge);
}

QByteArray KNTLM::ntlmv2Hash (const QString &target, const QString &user, const QString &password)
{
    const QByteArray hash = ntlmHash (password);
    const QByteArray key = QString2UnicodeLE (user.toUpper() + target);
    return hmacMD5 (key, hash);
}

QByteArray KNTLM::lmv2Response (const QByteArray &hash,
                                const QByteArray &clientData, const unsigned char *challenge)
{
    QByteArray data (8 + clientData.size(), 0);
    memcpy (data.data(), challenge, 8);
    memcpy (data.data() + 8, clientData.data(), clientData.size());

    QByteArray mac = hmacMD5 (data, hash);
    mac.resize (16 + clientData.size());
    memcpy (mac.data() + 16, clientData.data(), clientData.size());
    return mac;
}
