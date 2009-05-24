/*
   This file is part of the KDE libraries

   Copyright (c) 2007, 2008 Andreas Hartmetz <ahartmetz@gmail.com>

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
#ifndef KSSLD_H
#define KSSLD_H

#include <kdedmodule.h>
#include <QtCore/QList>
#include <QtCore/QVariant>


class KSSLDPrivate;
class KSslCertificateRule;
class QSslCertificate;
class QString;

class KSSLD : public KDEDModule
{
    Q_OBJECT
public:
    KSSLD(QObject *parent, const QVariantList &);
    ~KSSLD();

    void setRule(const KSslCertificateRule &rule);
    void clearRule(const KSslCertificateRule &rule);
    void clearRule(const QSslCertificate &cert, const QString &hostName);
    KSslCertificateRule rule(const QSslCertificate &cert, const QString &hostName) const;

    void setRootCertificates(const QList<QSslCertificate> &rootCertificates);
    QList<QSslCertificate> rootCertificates() const;
private:
    //AFAICS we don't need the d-pointer technique here but it makes the code look
    //more like the rest of kdelibs and it can be reused anywhere in kdelibs.
    KSSLDPrivate *const d;
};

#endif //KSSLD_H
