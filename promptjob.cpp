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

#include "promptjob.h"
#include "dbusbackend.h"

#include "prompt_interface.h"

#include <kdebug.h>


PromptJob::PromptJob( const QDBusObjectPath &path, const WId &parentId, QObject *parent ) : 
            KJob( parent ),
            promptPath( path ),
            parentWindowId( parentId )
{
}

void PromptJob::start()
{
    promptIf = DBusSession::createPrompt( promptPath );
    if ( promptIf->isValid() ) {
        connect( promptIf, SIGNAL(Completed(bool,const QDBusVariant&)), this, SLOT(promptCompleted(bool,const QDBusVariant&)) );
        // TODO: place a timer here to avoid hanging up if the prompt never calls promptCompleted
        // NOTE: however, care should be taken to avoid problems when user is too slow interacting with the prompt.
        //       a sensible timeout value should be chosen
        
        QDBusPendingReply<> promptReply = promptIf->Prompt( QString("%1").arg( parentWindowId ) );
        // NOTE: ne need to wait for promptReply to finish. The prompt will call promptCompleted when user interaction takes end
    }
    else {
        kDebug() << "ERROR instantiating prompt " << promptPath.path();
        setError(1); // FIXME: use enumerated error codes here
        setErrorText( QString("ERROR instantiating prompt with path '%1'").arg( promptPath.path() ) );
        emitResult();
    }
}

void PromptJob::promptCompleted(bool dism, const QDBusVariant &res)
{
    kDebug() << "dismissed = " << dism << ", result = " << res.variant().toString();
    dismissed = dism;
    opResult = res;
    setError(0);
    emitResult();
}



#include "promptjob.moc"
