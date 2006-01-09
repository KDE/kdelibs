/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include <qstring.h>
#include <qstringlist.h>
#include "ksslcertificate.h"
#include "ksslsigners.h"
#include <stdlib.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdatastream.h>


KSSLSigners::KSSLSigners() {
	dcc = new DCOPClient;
	dcc->attach();
}


KSSLSigners::~KSSLSigners() {
	delete dcc;
}

bool KSSLSigners::addCA(KSSLCertificate& cert,
                        bool ssl,
                        bool email,
                        bool code) const {
	return addCA(cert.toString(), ssl, email, code);
}


bool KSSLSigners::addCA(const QString &cert,
                        bool ssl,
                        bool email,
                        bool code) const {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     arg << ssl << email << code;
     bool rc = dcc->call("kded", "kssld",
                         "caAdd(QString,bool,bool,bool)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::regenerate() const {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     bool rc = dcc->call("kded", "kssld",
                         "caRegenerate()",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForSSL(KSSLCertificate& cert) const {
	return useForSSL(cert.getSubject());
}


bool KSSLSigners::useForSSL(const QString &subject) const{
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForSSL(QString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForEmail(KSSLCertificate& cert) const{
	return useForEmail(cert.getSubject());
}


bool KSSLSigners::useForEmail(const QString &subject) const{
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForEmail(QString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForCode(KSSLCertificate& cert) const{
	return useForCode(cert.getSubject());
}


bool KSSLSigners::useForCode(const QString &subject) const{
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForCode(QString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::remove(KSSLCertificate& cert) const {
	return remove(cert.getSubject());
}


bool KSSLSigners::remove(const QString &subject) const{
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caRemove(QString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


QStringList KSSLSigners::list() {
     QStringList drc;
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     bool rc = dcc->call("kded", "kssld",
                         "caList()",
                         data, rettype, retval);

     if (rc && rettype == "QStringList") {
        QDataStream retStream(retval);
        retStream >> drc;
     }

return drc;
}


QString KSSLSigners::getCert(const QString &subject) {
     QString drc;
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caGetCert(QString)",
                         data, rettype, retval);

     if (rc && rettype == "QString") {
        QDataStream retStream(retval);
        retStream >> drc;
     }

return drc;
}


bool KSSLSigners::setUse(const QString &subject, bool ssl, bool email, bool code) const {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << subject << ssl << email << code;
     bool rc = dcc->call("kded", "kssld",
                         "caSetUse(QString,bool,bool,bool)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}




