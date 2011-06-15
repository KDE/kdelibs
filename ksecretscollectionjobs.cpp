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

#include "ksecretscollectionjobs.h"

namespace KSecretsService {
    
CollectionJob::CollectionJob(Collection *collection, QObject* parent) : 
            KCompositeJob( parent ), 
            cd( collection->d ) {
    Q_ASSERT( cd != 0 );
    // the collection job must work on a valid collection from the backend (currently dbus)
    // if its not yed valid, then add the necessary subjob to get it connected
    if ( ! cd->isValid() ) {
        FindCollectionJob *findJob = new FindCollectionJob( collection, cd->collectioName, cd->findOptions, this );
        connect( findJob, SIGNAL(finished(KJob*)), this, SLOT(findCollectionFinished(KJob*)) );
        addSubjob( findJob );
    }
}


} // namespace

#include "ksecretscollectionjobs.moc"
