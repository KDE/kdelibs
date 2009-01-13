/* This file is part of the KDE libraries
    Copyright (C) 2008, 2009 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "httpauthentication.h"


// keys on even indexes, values on odd indexes. Reduces code expansion for the templated
// alternatives.
static QList<QByteArray> parseChallenge(const QByteArray &ba, QByteArray *scheme)
{
    QList<QByteArray> values;
    const int len = ba.count();
    const char *b = ba.constData();
    int start = 0;
    int end = 0;

    // parse scheme
    while (start < len && (b[start] == ' ' || b[start] == '\t')) {
        start++;
    }
    end = start;
    while (end < len && (b[end] != ' ' && b[end] != '\t')) {
        end++;
    }
    if (scheme) {
        *scheme = QByteArray(b + start, end - start);
    }

    while (end < len) {
        start = end;
        // parse key
        while (start < len && (b[start] == ' ' || b[start] == '\t')) {
            start++;
        }
        end = start;
        while (end < len && b[end] != '=') {
            end++;
        }
        values.append(QByteArray(b + start, end - start));
        if (end == len) {
            break;
        }

        // parse value
        start = end + 1;    //skip '='
        while (start < len && (b[start] == ' ' || b[start] == '\t')) {
            start++;
        }
        if (start + 1 < len && b[start] == '"') {
            end = ++start;
            //quoted string
            while (end < len && b[end] != '"') {
                end++;
            }
            values.append(QByteArray(b + start, end - start));
            //the quoted string has ended, but only a comma ends a key-value pair
            while (end < len && b[end] != ',') {
                end++;
            }
        } else {
            end = start;
            //unquoted string
            while (end < len && b[end] != ',') {
                end++;
            }
            values.append(QByteArray(b + start, end - start));
        }
        // end may point beyond the buffer already here
        end++;  // skip comma
    }
    // ensure every key has a value
    if (values.count() % 2) {
        values.removeLast();
    }
    return values;
}


static QByteArray valueForKey(const QList<QByteArray> &ba, const QByteArray &key)
{
    for (int i = 0; i + 1 < ba.count(); i += 2) {
        if (ba[i] == key) {
            return ba[i + 1];
        }
    }
    return QByteArray();
}


QByteArray KAbstractHttpAuthentication::bestOffer(const QList<QByteArray> &offers)
{
    // choose the most secure auth scheme offered
    QByteArray digestOffer;
    QByteArray ntlmOffer;
    QByteArray basicOffer;
    foreach (const QByteArray &offer, offers) {
        QByteArray scheme = offer.mid(0, 10).toLower();
        if (scheme.startsWith("digest")) {
            digestOffer = offer;
        } else if (scheme.startsWith("ntlm")) {
            ntlmOffer = offer;
        } else if (scheme.startsWith("basic")) {
            basicOffer = offer;
        }
    }

    if (!digestOffer.isEmpty()) {
        return digestOffer;
    } else if (!ntlmOffer.isEmpty()) {
        return ntlmOffer;
    }
    return basicOffer;  //empty or not...
}


KAbstractHttpAuthentication *KAbstractHttpAuthentication::newAuth(const QByteArray &offer)
{
    QByteArray scheme = offer.mid(0, 10).toLower();
    if (scheme.startsWith("digest")) {
        return new KHttpDigestAuthentication();
    } else if (scheme.startsWith("ntlm")) {
        return new KHttpNtlmAuthentication();
    } else if (scheme.startsWith("basic")) {
        return new KHttpBasicAuthentication();
    }
    return 0;
}


void KAbstractHttpAuthentication::reset()
{
    m_scheme.clear();
    m_challenge.clear();
    m_resource.clear();
    m_httpMethod.clear();
    m_isError = false;
    m_retryWithSameCredentials = false;
    m_forceKeepAlive = false;
    m_forceDisconnect = false;
    m_headerFragment.clear();
    m_username.clear();
    m_password.clear();
}


void KAbstractHttpAuthentication::setChallenge(const QByteArray &c, const KUrl &resource,
                                               const QByteArray &httpMethod)
{
    reset();
    m_challenge = parseChallenge(c, &m_scheme);
    Q_ASSERT(m_scheme.toLower() == scheme().toLower());
    m_resource = resource;
    m_httpMethod = httpMethod;
}


QString KAbstractHttpAuthentication::realm() const
{
    QByteArray realm = valueForKey(m_challenge, "realm");
    if (KGlobal::locale()->language().contains("ru")) {
        //for sites like lib.homelinux.org
        return QTextCodec::codecForName("CP1251")->toUnicode(realm);
    }
    return QString::fromLatin1(realm);
}


void KAbstractHttpAuthentication::authInfoBoilerplate(KIO::AuthInfo *a) const
{
    a->verifyPath = true;  //### research this
    a->url = m_resource;
    a->realmValue = realm();
    a->username = m_username;
    a->password = m_password;
}


void KAbstractHttpAuthentication::generateResponseCommon(const QString &user, const QString &password)
{
    if (user.isEmpty() || password.isEmpty() || m_scheme.isEmpty() || m_httpMethod.isEmpty()) {
        m_isError = true;
        return;
    }

    m_username = user;
    m_password = password;
    m_isError = false;
    m_retryWithSameCredentials = false;
    m_forceKeepAlive = false;
    m_forceDisconnect = false;
}


QByteArray KHttpBasicAuthentication::scheme() const
{
    return "Basic";
}


void KHttpBasicAuthentication::fillKioAuthInfo(KIO::AuthInfo *ai) const
{
    authInfoBoilerplate(ai);
}


void KHttpBasicAuthentication::generateResponse(const QString &user, const QString &password)
{
    generateResponseCommon(user, password);
    if (m_isError) {
        return;
    }

    m_headerFragment = "Basic ";
    m_headerFragment += KCodecs::base64Encode(user.toLatin1() + ':' + password.toLatin1());
    m_headerFragment += "\r\n";
    return;
}


QByteArray KHttpDigestAuthentication::scheme() const
{
    return "Digest";
}


void KHttpDigestAuthentication::fillKioAuthInfo(KIO::AuthInfo *ai) const
{
    authInfoBoilerplate(ai);
}


struct DigestAuthInfo
{
    QByteArray nc;
    QByteArray qop;
    QByteArray realm;
    QByteArray nonce;
    QByteArray method;
    QByteArray cnonce;
    QByteArray username;
    QByteArray password;
    KUrl::List digestURIs;
    QByteArray algorithm;
    QByteArray entityBody;
};


//calculateResponse() from the original HTTPProtocol
static QByteArray calculateResponse(const DigestAuthInfo &info, const KUrl &resource)
{
  kDebug(7113) << info.nc << info.qop << info.realm << info.nonce << info.method << info.cnonce
               << info.username << info.password << info.algorithm << info.entityBody;
  foreach (const KUrl &u, info.digestURIs) {
      kDebug(7113) << u;
  }
  kDebug(7113);
  KMD5 md;
  QByteArray HA1;
  QByteArray HA2;

  // Calculate H(A1)
  QByteArray authStr = info.username;
  authStr += ':';
  authStr += info.realm;
  authStr += ':';
  authStr += info.password;
  md.update( authStr );

  if ( info.algorithm.toLower() == "md5-sess" )
  {
    authStr = md.hexDigest();
    authStr += ':';
    authStr += info.nonce;
    authStr += ':';
    authStr += info.cnonce;
    md.reset();
    md.update( authStr );
  }
  HA1 = md.hexDigest();

  kDebug(7113) << "A1 => " << HA1;

  // Calcualte H(A2)
  authStr = info.method;
  authStr += ':';
  authStr += resource.encodedPathAndQuery(KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath).toLatin1();
  if ( info.qop == "auth-int" )
  {
    authStr += ':';
    authStr += info.entityBody;
  }
  md.reset();
  md.update( authStr );
  HA2 = md.hexDigest();

  kDebug(7113) << "A2 => " << HA2;

  // Calcualte the response.
  authStr = HA1;
  authStr += ':';
  authStr += info.nonce;
  authStr += ':';
  if ( !info.qop.isEmpty() )
  {
    authStr += info.nc;
    authStr += ':';
    authStr += info.cnonce;
    authStr += ':';
    authStr += info.qop;
    authStr += ':';
  }
  authStr += HA2;
  md.reset();
  md.update( authStr );

  QByteArray Response = md.hexDigest();

  kDebug(7113) << "Response => " << Response;
  return Response;
}


void KHttpDigestAuthentication::generateResponse(const QString &user, const QString &password)
{
    generateResponseCommon(user, password);
    if (m_isError) {
        return;
    }

// magic starts here (this part is slightly modified from the original in HTTPProtocol)

    DigestAuthInfo info;

    info.username = user.toLatin1();        //### charset breakage
    info.password = password.toLatin1();    //###

    // info.entityBody = p;  // FIXME: send digest of data for POST action ??
    info.realm = "";
    info.nonce = "";
    info.qop = "";

    // cnonce is recommended to contain about 64 bits of entropy
    info.cnonce = KRandom::randomString(16).toLatin1();

    // HACK: Should be fixed according to RFC 2617 section 3.2.2
    info.nc = "00000001";

    // Set the method used...
    info.method = m_httpMethod;

    // Parse the Digest response....
    if (valueForKey(m_challenge, "stale").toLower() == "true") {
        m_retryWithSameCredentials = true;
    }

    info.realm = valueForKey(m_challenge, "realm");

    info.algorithm = valueForKey(m_challenge, "algorith");
    if (info.algorithm.isEmpty()) {
        info.algorithm = valueForKey(m_challenge, "algorithm");
    }
    if (info.algorithm.isEmpty()) {
        info.algorithm = "MD5";
    }

    foreach (const QByteArray &path, valueForKey(m_challenge, "domain").split(' ')) {
        KUrl u(m_resource, QString::fromLatin1(path));
        if (u.isValid()) {
            info.digestURIs.append(u);
        }
    }

    info.nonce = valueForKey(m_challenge, "nonce");
    QByteArray opaque = valueForKey(m_challenge, "opaque");
    info.qop = valueForKey(m_challenge, "qop");

    if (info.realm.isEmpty() || info.nonce.isEmpty()) {
        // ### proper error return
        m_isError = true;
        return;
    }

  // If the "domain" attribute was not specified and the current response code
  // is authentication needed, add the current request url to the list over which
  // this credential can be automatically applied.
  if (info.digestURIs.isEmpty() /*###&& (m_request.responseCode == 401 || m_request.responseCode == 407)*/)
    info.digestURIs.append (m_resource);
  else
  {
    // Verify whether or not we should send a cached credential to the
    // server based on the stored "domain" attribute...
    bool send = true;

    // Determine the path of the request url...
    QString requestPath = m_resource.directory(KUrl::AppendTrailingSlash | KUrl::ObeyTrailingSlash);
    if (requestPath.isEmpty())
      requestPath = "/";

    foreach (const KUrl &u, info.digestURIs)
    {
      send &= (m_resource.protocol().toLower() == u.protocol().toLower());
      send &= (m_resource.host().toLower() == u.host().toLower());

      if (m_resource.port() > 0 && u.port() > 0)
        send &= (m_resource.port() == u.port());

      QString digestPath = u.directory (KUrl::AppendTrailingSlash | KUrl::ObeyTrailingSlash);
      if (digestPath.isEmpty())
        digestPath = "/";

      send &= (requestPath.startsWith(digestPath));

      if (send)
        break;
    }

    kDebug(7113) << "passed digest authentication credential test: " << send;

    if (!send) {
        m_isError = true;
        return;
    }
  }

  kDebug(7113) << "RESULT OF PARSING:";
  kDebug(7113) << "  algorithm: " << info.algorithm;
  kDebug(7113) << "  realm:     " << info.realm;
  kDebug(7113) << "  nonce:     " << info.nonce;
  kDebug(7113) << "  opaque:    " << opaque;
  kDebug(7113) << "  qop:       " << info.qop;

  // Calculate the response...
  QByteArray Response = calculateResponse(info, m_resource);

  QString auth = "Digest username=\"";
  auth += info.username;

  auth += "\", realm=\"";
  auth += info.realm;
  auth += "\"";

  auth += ", nonce=\"";
  auth += info.nonce;

  auth += "\", uri=\"";
  auth += m_resource.encodedPathAndQuery(KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath);

  if (!info.algorithm.isEmpty()) {
    auth += "\", algorithm=\"";
    auth += info.algorithm;
    auth +="\"";
  }

  if ( !info.qop.isEmpty() )
  {
    auth += ", qop=\"";
    auth += info.qop;
    auth += "\", cnonce=\"";
    auth += info.cnonce;
    auth += "\", nc=";
    auth += info.nc;
  }

  auth += ", response=\"";
  auth += Response;
  if ( !opaque.isEmpty() )
  {
    auth += "\", opaque=\"";
    auth += opaque;
  }
  auth += "\"\r\n";

// magic ends here
    // note that auth already contains \r\n
    m_headerFragment = auth;
    return;
}


QByteArray KHttpNtlmAuthentication::scheme() const
{
    return "NTLM";
}


void KHttpNtlmAuthentication::fillKioAuthInfo(KIO::AuthInfo *ai) const
{
    authInfoBoilerplate(ai);
    // Every auth scheme is supposed to supply a realm according to the RFCs. Of course this doesn't
    // prevent Microsoft from not doing it... Dummy value!
    ai->realmValue = "NTLM";
}


void KHttpNtlmAuthentication::generateResponse(const QString &_user, const QString &password)
{
    generateResponseCommon(_user, password);
    if (m_isError) {
        return;
    }

    QByteArray buf;
    
    // ### double check if forceDisconnect and forceKeepAlive are a) necessary b) set correctly here
    if (m_challenge.isEmpty()) {
        // first, send type 1 message (with empty domain, workstation..., but it still works)
        m_forceDisconnect = true;
        KNTLM::getNegotiate(buf);
    } else {
        // we've (hopefully) received a valid type 2 message: send type 3 message as last step
        QString domain;
        QString user = _user;
        if (user.contains('\\')) {
            domain = user.section('\\', 0, 0);
            user = user.section('\\', 1);
        }

        m_retryWithSameCredentials = true;
        m_forceKeepAlive = true;
        QByteArray challenge;
        KCodecs::base64Decode(m_challenge[0], challenge);
        KNTLM::getAuth(buf, challenge, user, password, domain, QHostInfo::localHostName());
    }

    m_headerFragment += KCodecs::base64Encode(buf);
    m_headerFragment += "\r\n";

    return;
}
