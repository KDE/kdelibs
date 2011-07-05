/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETSSERVICEITEMJOBS_H
#define KSECRETSSERVICEITEMJOBS_H

#include "ksecretsservicesecret.h"

#include <kjob.h>
#include <QSharedPointer>

class GetSecretJobPrivate;

namespace KSecretsService {
    
class GetSecretJob : public KJob {
    Q_OBJECT
    Q_DISABLE_COPY(GetSecretJob)
public:
    
    Secret secret() const;
    
private:
    friend class ::GetSecretJobPrivate;
    QSharedPointer< GetSecretJobPrivate > d;
};

}; // namespace

#endif // KSECRETSSERVICEITEMJOBS_H
