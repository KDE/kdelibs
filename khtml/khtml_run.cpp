/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
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
#include "khtmlpart_p.h"
#include "khtml_run.h"
#include <kio/job.h>
#include <kdebug.h>
#include <klocale.h>
#include "khtml_ext.h"
#include <qwidget.h>

KHTMLRun::KHTMLRun( KHTMLPart *part, khtml::ChildFrame *child, const KUrl &url,
                    const KParts::URLArgs &args, bool hideErrorDialog )
    : KParts::BrowserRun( url, args, part, part->widget() ? part->widget()->topLevelWidget() : 0,
                          false, false, hideErrorDialog ),
  m_child( child )
{
    // Don't use an external browser for parts of a webpage we are rendering. (iframes at least are one example)
    setEnableExternalBrowser(false);

    // get the wheel to start spinning
    part->started(0L);
}

//KHTMLPart *KHTMLRun::htmlPart() const
//{ return static_cast<KHTMLPart *>(m_part); }

void KHTMLRun::foundMimeType( const QString &_type )
{
    Q_ASSERT(!m_bFinished);
    QString mimeType = _type; // this ref comes from the job, we lose it when using KIO again
    if ( static_cast<KHTMLPart *>(m_part)->processObjectRequest( m_child, m_strURL, mimeType ) )
        m_bFinished = true;
    else {
        if ( m_bFinished ) // abort was called (this happens with the activex fallback for instance)
            return;
        // Couldn't embed -> call BrowserRun::handleNonEmbeddable()
        KParts::BrowserRun::NonEmbeddableResult res = handleNonEmbeddable( mimeType );
        if ( res == KParts::BrowserRun::Delayed )
            return;
        m_bFinished = ( res == KParts::BrowserRun::Handled );
        if ( m_bFinished ) { // saved or canceled -> flag completed
            m_child->m_bCompleted = true;
            static_cast<KHTMLPart *>(m_part)->checkCompleted();
        }
    }

    if ( m_bFinished )
    {
        m_timer.start( 0, true );
        return;
    }

    //kDebug(6050) << "KHTMLRun::foundMimeType " << _type << " couldn't open" << endl;
    KRun::foundMimeType( mimeType );

    // "open" is finished -> flag completed
    m_child->m_bCompleted = true;
    static_cast<KHTMLPart *>(m_part)->checkCompleted();
}

void KHTMLRun::save( const KUrl & url, const QString & suggestedFilename )
{
    KHTMLPopupGUIClient::saveURL( m_part->widget(), i18n( "Save As" ), url, m_args.metaData(), QString(), 0, suggestedFilename );
}

#include "khtml_run.moc"
