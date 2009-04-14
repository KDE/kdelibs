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
#include <kio/previewjob.h>

#include <QtGui/QLayout>

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
  : KPropertiesDialogPlugin(props),d(new KPreviewPropsPluginPrivate)
{

    if (properties->items().count()>1)
        return;

    createLayout();
}

void KPreviewPropsPlugin::createLayout()
{
    // let the dialog create the page frame
    QFrame* topframe = new QFrame();
    properties->addPage(topframe, i18n("P&review"));
    topframe->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout* tmp = new QVBoxLayout(topframe);
    tmp->setMargin(0);

    preview = new KFileMetaPreview(topframe);

    tmp->addWidget(preview) ;
    connect( properties, SIGNAL( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ), SLOT( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ) );
}

KPreviewPropsPlugin::~KPreviewPropsPlugin()
{
    delete d;
}

bool KPreviewPropsPlugin::supports( const KFileItemList &_items )
{
    if ( _items.count() != 1 )
        return false;
    bool metaDataEnabled = KGlobalSettings::showFilePreview(_items.first().url());
    if (!metaDataEnabled)
        return false;
    const KMimeType::Ptr mime = _items.first().mimeTypePtr();
    const QStringList supportedMimeTypes = KIO::PreviewJob::supportedMimeTypes();
    foreach(const QString& supportedMime, supportedMimeTypes) {
        if (mime->is(supportedMime))
    return true;
    }
    return false;
}

void KPreviewPropsPlugin::currentPageChanged( KPageWidgetItem *current, KPageWidgetItem * )
{
    if ( current->widget() != preview->parent() )
        return;

    disconnect( properties, SIGNAL( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ), this, SLOT( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ) );
    preview->showPreview(properties->item().url());
}

#include "kpreviewprops.moc"
