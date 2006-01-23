/* This file is part of the KDE libraries
    Copyright (C) 2005 Stephan Binner <binner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

 */

#include "kpreviewprops.h"

#include <qlayout.h>

#include <kfilemetapreview.h>
#include <kglobalsettings.h>
#include <klocale.h>

class KPreviewPropsPlugin::KPreviewPropsPluginPrivate
{
public:
    KPreviewPropsPluginPrivate()  {}
    ~KPreviewPropsPluginPrivate() {}
};

KPreviewPropsPlugin::KPreviewPropsPlugin(KPropertiesDialog* props)
  : KPropsDlgPlugin(props),d(new KPreviewPropsPluginPrivate)
{

    if (properties->items().count()>1)
        return;

    createLayout();
}

void KPreviewPropsPlugin::createLayout()
{
    // let the dialog create the page frame
    QFrame* topframe = properties->addPage(i18n("P&review"));
    topframe->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout* tmp = new QVBoxLayout(topframe, 0, 0);

    preview = new KFileMetaPreview(topframe);

    tmp->addWidget(preview) ;
    connect( properties, SIGNAL( currentPageChanged( QWidget * ) ), SLOT( currentPageChanged( QWidget* ) ) );
}

KPreviewPropsPlugin::~KPreviewPropsPlugin()
{
    delete d;
}

bool KPreviewPropsPlugin::supports( KFileItemList _items )
{
    bool metaDataEnabled = KGlobalSettings::showFilePreview(_items.first()->url());
    KMimeType::Ptr mt = KMimeType::findByURL( _items.first()->url() );

    if ( _items.count() != 1 || !metaDataEnabled || mt->name() == "inode/directory" || mt->name().startsWith( "media/" ) )
        return false;

    //TODO Copy everything of KFileMetaPreview::previewProviderFor() ?

    return true;
}

void KPreviewPropsPlugin::currentPageChanged( QWidget* widget )
{
    if ( widget != preview->parent() )
        return;

    disconnect( properties, SIGNAL( currentPageChanged( QWidget * ) ), this, SLOT( currentPageChanged( QWidget* ) ) );
    preview->showPreview(properties->item()->url());
}

#include "kpreviewprops.moc"
