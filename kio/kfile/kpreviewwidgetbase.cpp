/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kpreviewwidgetbase.h"
#include <qstringlist.h>

class KPreviewWidgetBase::KPreviewWidgetBasePrivate
{
public:
    QStringList supportedMimeTypes;
};

QHash<KPreviewWidgetBase*, KPreviewWidgetBase::KPreviewWidgetBasePrivate*> * KPreviewWidgetBase::s_private;

KPreviewWidgetBase::KPreviewWidgetBase( QWidget *parent )
    : QWidget( parent )
{
    if ( !s_private )
        s_private = new QHash<KPreviewWidgetBase*, KPreviewWidgetBasePrivate*>();

    s_private->insert( this, new KPreviewWidgetBasePrivate() );
}

KPreviewWidgetBase::~KPreviewWidgetBase()
{
    delete s_private->take( this );
    if ( s_private->isEmpty() )
    {
        delete s_private;
        s_private = 0L;
    }
}

void KPreviewWidgetBase::setSupportedMimeTypes( const QStringList& mimeTypes )
{
    d()->supportedMimeTypes = mimeTypes;
}

QStringList KPreviewWidgetBase::supportedMimeTypes() const
{
    return d()->supportedMimeTypes;
}

#include "kpreviewwidgetbase.moc"
