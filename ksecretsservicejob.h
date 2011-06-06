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

#ifndef KSECRETSJOB_H
#define KSECRETSJOB_H

#include <kjob.h>

namespace KSecretsService {

/**
 * This is the base class for the SecretsJob template class.
 * It's rationale is to get signals and slots on it, as moc
 * won't support template classes. @see SecretsJob
 */
class SecretsJobBase : public KJob {
    Q_OBJECT
    Q_DISABLE_COPY(SecretsJobBase)
public:
    explicit SecretsJobBase( QObject *parent =0 );
    ~SecretsJobBase();
    
protected:
    class Private;
    Private *d;
};

};

#endif // KSECRETSJOB_H
