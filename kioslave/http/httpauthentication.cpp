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
static QList<QByteArray> parseChallenge(const QByteArray &ba, QByteArray *scheme = 0)
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
    // WARNING: Do not remove the > 1 check or parsing a Type 1 NTLM
    // authentication challenge will surely fail.
    if (values.count() > 1 && values.count() % 2) {
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

KAbstractHttpAuthentication::KAbstractHttpAuthentication(KConfigGroup *config)
                            :m_config(config), m_finalAuthStage(true)
{
    reset();
}

KAbstractHttpAuthentication::~KAbstractHttpAuthentication()
{
}

QByteArray KAbstractHttpAuthentication::bestOffer(const QList<QByteArray> &offers)
{
    // choose the most secure auth scheme offered
    QByteArray negotiateOffer;
    QByteArray digestOffer;
    QByteArray ntlmOffer;
    QByteArray basicOffer;
    Q_FOREACH (const QByteArray &offer, offers) {
        QByteArray scheme = offer.mid(0, 10).toLower();
#ifdef HAVE_LIBGSSAPI
        if (scheme.startsWith("negotiate")) { // krazy:exclude=strings
            negotiateOffer = offer;
        } else
#endif
        if (scheme.startsWith("digest")) { // krazy:exclude=strings
            digestOffer = offer;
        } else if (scheme.startsWith("ntlm")) { // krazy:exclude=strings
            ntlmOffer = offer;
        } else if (scheme.startsWith("basic")) { // krazy:exclude=strings
            basicOffer = offer;
        }
    }

    if (!negotiateOffer.isEmpty()) {
        return negotiateOffer;
    } else if (!digestOffer.isEmpty()) {
        return digestOffer;
    } else if (!ntlmOffer.isEmpty()) {
        return ntlmOffer;
    }
    return basicOffer;  //empty or not...
}


KAbstractHttpAuthentication *KAbstractHttpAuthentication::newAuth(const QByteArray &offer, KConfigGroup* config)
{
    QByteArray scheme = offer.mid(0, 10).toLower();
#ifdef HAVE_LIBGSSAPI
    if (scheme.startsWith("negotiate")) { // krazy:exclude=strings
        return new KHttpNegotiateAuthentication(config);
    } else
#else
    Q_UNUSED(config);
#endif
    if (scheme.startsWith("digest")) { // krazy:exclude=strings
        return new KHttpDigestAuthentication();
    } else if (scheme.startsWith("ntlm")) { // krazy:exclude=strings
        return new KHttpNtlmAuthentication();
    } else if (scheme.startsWith("basic")) { // krazy:exclude=strings
        return new KHttpBasicAuthentication();
    }
    return 0;
}


void KAbstractHttpAuthentication::reset()
{
    m_scheme.clear();
    m_challenge.clear();
    m_challengeText.clear();
    m_resource.clear();
    m_httpMethod.clear();
    m_isError = false;
    m_needCredentials = true;
    m_forceKeepAlive = false;
    m_forceDisconnect = false;
    m_headerFragment.clear();
    m_username.clear();
    m_password.clear();
    m_config = 0;
}

void KAbstractHttpAuthentication::setChallenge(const QByteArray &c, const KUrl &resource,
                                               const QByteArray &httpMethod)
{
    reset();
    m_challengeText = c.trimmed();
    m_challenge = parseChallenge(m_challengeText, &m_scheme);
    Q_ASSERT(m_scheme.toLower() == scheme().toLower());
    m_resource = resource;
    m_httpMethod = httpMethod.trimmed();
}


QString KAbstractHttpAuthentication::realm() const
{
    const QByteArray realm = valueForKey(m_challenge, "realm");
    if (KGlobal::locale()->language().contains(QLatin1String("ru"))) {
        //for sites like lib.homelinux.org
        return QTextCodec::codecForName("CP1251")->toUnicode(realm);
    }
    return QString::fromLatin1(realm);
}

void KAbstractHttpAuthentication::authInfoBoilerplate(KIO::AuthInfo *a) const
{
    a->url = m_resource;
    a->username = m_username;
    a->password = m_password;
    a->verifyPath = supportsPathMatching();
    a->realmValue = realm();
    a->digestInfo = QLatin1String(authDataToCache());
}


void KAbstractHttpAuthentication::generateResponseCommon(const QString &user, const QString &password)
{
    if (m_scheme.isEmpty() || m_httpMethod.isEmpty()) {
        m_isError = true;
        return;
    }

    if (m_needCredentials) {
        m_username = user;
        m_password = password;
    }

    m_isError = false;
    // we could leave m_needCredentials value alone; this is just defensive coding.
    m_needCredentials = true;
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
    m_headerFragment += QByteArray(m_username.toLatin1() + ':' + m_password.toLatin1()).toBase64();
    m_headerFragment += "\r\n";
}


QByteArray KHttpDigestAuthentication::scheme() const
{
    return "Digest";
}


void KHttpDigestAuthentication::setChallenge(const QByteArray &c, const KUrl &resource,
                                             const QByteArray &httpMethod)
{
    QString oldUsername;
    QString oldPassword;
    if (valueForKey(m_challenge, "stale").toLower() == "true") {
        // stale nonce: the auth failure that triggered this round of authentication is an artifact
        // of digest authentication. the credentials are probably still good, so keep them.
        oldUsername = m_username;
        oldPassword = m_password;
    }
    KAbstractHttpAuthentication::setChallenge(c, resource, httpMethod);
    if (!oldUsername.isEmpty() && !oldPassword.isEmpty()) {
        // keep credentials *and* don't ask for new ones
        m_needCredentials = false;
        m_username = oldUsername;
        m_password = oldPassword;
    }
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

  const QByteArray response = md.hexDigest();
  kDebug(7113) << "Response =>" << response;
  return response;
}


void KHttpDigestAuthentication::generateResponse(const QString &user, const QString &password)
{
    generateResponseCommon(user, password);
    if (m_isError) {
        return;
    }

    // magic starts here (this part is slightly modified from the original in HTTPProtocol)

    DigestAuthInfo info;

    info.username = m_username.toLatin1();  //### charset breakage
    info.password = m_password.toLatin1();  //###

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
    info.realm = valueForKey(m_challenge, "realm");

    info.algorithm = valueForKey(m_challenge, "algorithm");
    if (info.algorithm.isEmpty()) {
        info.algorithm = valueForKey(m_challenge, "algorith");
    }
    if (info.algorithm.isEmpty()) {
        info.algorithm = "MD5";
    }

    Q_FOREACH (const QByteArray &path, valueForKey(m_challenge, "domain").split(' ')) {
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
          requestPath = QLatin1Char('/');

        Q_FOREACH (const KUrl &u, info.digestURIs)
        {
          send &= (m_resource.protocol().toLower() == u.protocol().toLower());
          send &= (m_resource.host().toLower() == u.host().toLower());

          if (m_resource.port() > 0 && u.port() > 0)
            send &= (m_resource.port() == u.port());

          QString digestPath = u.directory (KUrl::AppendTrailingSlash | KUrl::ObeyTrailingSlash);
          if (digestPath.isEmpty())
            digestPath = QLatin1Char('/');

          send &= (requestPath.startsWith(digestPath));

          if (send)
            break;
        }

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
    const QByteArray response = calculateResponse(info, m_resource);

    QByteArray auth = "Digest username=\"";
    auth += info.username;

    auth += "\", realm=\"";
    auth += info.realm;
    auth += "\"";

    auth += ", nonce=\"";
    auth += info.nonce;

    auth += "\", uri=\"";
    auth += m_resource.encodedPathAndQuery(KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath).toLatin1();

    if (!info.algorithm.isEmpty()) {
      auth += "\", algorithm=";
      auth += info.algorithm;
    }

    if ( !info.qop.isEmpty() )
    {
      auth += ", qop=";
      auth += info.qop;
      auth += ", cnonce=\"";
      auth += info.cnonce;
      auth += "\", nc=";
      auth += info.nc;
    }

    auth += ", response=\"";
    auth += response;
    if ( !opaque.isEmpty() )
    {
      auth += "\", opaque=\"";
      auth += opaque;
    }
    auth += "\"\r\n";

    // magic ends here
    // note that auth already contains \r\n
    m_headerFragment = auth;
}


QByteArray KHttpNtlmAuthentication::scheme() const
{
    return "NTLM";
}


void KHttpNtlmAuthentication::setChallenge(const QByteArray &c, const KUrl &resource,
                                             const QByteArray &httpMethod)
{
    KAbstractHttpAuthentication::setChallenge(c, resource, httpMethod);
    if (m_challenge.isEmpty()) {
        // The type 1 message we're going to send needs no credentials;
        // they come later in the type 3 message.
        m_needCredentials = false;
    }
}


void KHttpNtlmAuthentication::fillKioAuthInfo(KIO::AuthInfo *ai) const
{
    authInfoBoilerplate(ai);
    // Every auth scheme is supposed to supply a realm according to the RFCs. Of course this doesn't
    // prevent Microsoft from not doing it... Dummy value!
    // we don't have the username yet which may (may!) contain a domain, so we really have no choice
    ai->realmValue = QLatin1String("NTLM");
}


void KHttpNtlmAuthentication::generateResponse(const QString &_user, const QString &password)
{
    generateResponseCommon(_user, password);
    if (m_isError) {
        return;
    }

    QByteArray buf;

    if (m_challenge.isEmpty()) {
        m_finalAuthStage = false;
        // first, send type 1 message (with empty domain, workstation..., but it still works)
        if (!KNTLM::getNegotiate(buf)) {
            kWarning(7113) << "Error while constructing Type 1 NTLM authentication request";
            m_isError = true;
            return;
        }
    } else {
        m_finalAuthStage = true;
        // we've (hopefully) received a valid type 2 message: send type 3 message as last step
        QString user, domain;
        if (m_username.contains(QLatin1Char('\\'))) {
            domain = m_username.section(QLatin1Char('\\'), 0, 0);
            user = m_username.section(QLatin1Char('\\'), 1);
        } else {
            user = m_username;
        }

        m_forceKeepAlive = true;
        const QByteArray challenge = QByteArray::fromBase64(m_challenge[0]);
        if (!KNTLM::getAuth(buf, challenge, user, password, domain, QHostInfo::localHostName())) {
            kWarning(7113) << "Error while constructing Type 3 NTLM authentication request";
            m_isError = true;
            return;
        }
    }

    m_headerFragment = "NTLM ";
    m_headerFragment += buf.toBase64();
    m_headerFragment += "\r\n";

    return;
}


//////////////////////////
#ifdef HAVE_LIBGSSAPI

// just an error message formatter
static QByteArray gssError(int major_status, int minor_status)
{
    OM_uint32 new_status;
    OM_uint32 msg_ctx = 0;
    gss_buffer_desc major_string;
    gss_buffer_desc minor_string;
    OM_uint32 ret;
    QByteArray errorstr;

    do {
        ret = gss_display_status(&new_status, major_status, GSS_C_GSS_CODE, GSS_C_NULL_OID, &msg_ctx, &major_string);
        errorstr += (const char *)major_string.value;
        errorstr += ' ';
        ret = gss_display_status(&new_status, minor_status, GSS_C_MECH_CODE, GSS_C_NULL_OID, &msg_ctx, &minor_string);
        errorstr += (const char *)minor_string.value;
        errorstr += ' ';
    } while (!GSS_ERROR(ret) && msg_ctx != 0);

    return errorstr;
}


QByteArray KHttpNegotiateAuthentication::scheme() const
{
    return "Negotiate";
}


void KHttpNegotiateAuthentication::setChallenge(const QByteArray &c, const KUrl &resource,
                                                const QByteArray &httpMethod)
{
    KAbstractHttpAuthentication::setChallenge(c, resource, httpMethod);
    // GSSAPI knows how to get the credentials on its own
    m_needCredentials = false;
}


void KHttpNegotiateAuthentication::fillKioAuthInfo(KIO::AuthInfo *ai) const
{
    authInfoBoilerplate(ai);
    //### does GSSAPI supply anything realm-like? dummy value for now.
    ai->realmValue = QLatin1String("Negotiate");
}


void KHttpNegotiateAuthentication::generateResponse(const QString &user, const QString &password)
{
    generateResponseCommon(user, password);
    if (m_isError) {
        return;
    }

    OM_uint32 major_status, minor_status;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    gss_name_t server;
    gss_ctx_id_t ctx;
    gss_OID mech_oid;
    static gss_OID_desc krb5_oid_desc = {9, (void *) "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02"};
    static gss_OID_desc spnego_oid_desc = {6, (void *) "\x2b\x06\x01\x05\x05\x02"};
    gss_OID_set mech_set;
    gss_OID tmp_oid;

    ctx = GSS_C_NO_CONTEXT;
    mech_oid = &krb5_oid_desc;

    // see whether we can use the SPNEGO mechanism
    major_status = gss_indicate_mechs(&minor_status, &mech_set);
    if (GSS_ERROR(major_status)) {
        kDebug(7113) << "gss_indicate_mechs failed: " << gssError(major_status, minor_status);
    } else {
        for (uint i = 0; i < mech_set->count; i++) {
            tmp_oid = &mech_set->elements[i];
            if (tmp_oid->length == spnego_oid_desc.length &&
                !memcmp(tmp_oid->elements, spnego_oid_desc.elements, tmp_oid->length)) {
                kDebug(7113) << "found SPNEGO mech";
                mech_oid = &spnego_oid_desc;
                break;
            }
        }
        gss_release_oid_set(&minor_status, &mech_set);
    }

    // the service name is "HTTP/f.q.d.n"
    QByteArray servicename = "HTTP@";
    servicename += m_resource.host().toAscii();

    input_token.value = (void *)servicename.data();
    input_token.length = servicename.length() + 1;

    major_status = gss_import_name(&minor_status, &input_token,
                                   GSS_C_NT_HOSTBASED_SERVICE, &server);

    input_token.value = NULL;
    input_token.length = 0;

    if (GSS_ERROR(major_status)) {
        kDebug(7113) << "gss_import_name failed: " << gssError(major_status, minor_status);
        m_isError = true;
        return;
    }

    OM_uint32 req_flags;
    if (m_config && m_config->readEntry("DelegateCredentialsOn", false))
       req_flags = GSS_C_DELEG_FLAG;
    else
       req_flags = 0;

    // GSSAPI knows how to get the credentials its own way, so don't ask for any
    major_status = gss_init_sec_context(&minor_status, GSS_C_NO_CREDENTIAL,
                                        &ctx, server, mech_oid,
                                        req_flags, GSS_C_INDEFINITE,
                                        GSS_C_NO_CHANNEL_BINDINGS,
                                        GSS_C_NO_BUFFER, NULL, &output_token,
                                        NULL, NULL);

    if (GSS_ERROR(major_status) || (output_token.length == 0)) {
        kDebug(7113) << "gss_init_sec_context failed: " << gssError(major_status, minor_status);
        gss_release_name(&minor_status, &server);
        if (ctx != GSS_C_NO_CONTEXT) {
            gss_delete_sec_context(&minor_status, &ctx, GSS_C_NO_BUFFER);
            ctx = GSS_C_NO_CONTEXT;
        }
        m_isError = true;
        return;
    }

    m_headerFragment = "Negotiate ";
    m_headerFragment += QByteArray::fromRawData((const char *)output_token.value,
                                                output_token.length).toBase64();
    m_headerFragment += "\r\n";

    // free everything
    gss_release_name(&minor_status, &server);
    if (ctx != GSS_C_NO_CONTEXT) {
        gss_delete_sec_context(&minor_status, &ctx, GSS_C_NO_BUFFER);
        ctx = GSS_C_NO_CONTEXT;
    }
    gss_release_buffer(&minor_status, &output_token);
}

#endif // HAVE_LIBGSSAPI
