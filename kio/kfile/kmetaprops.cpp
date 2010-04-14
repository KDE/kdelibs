/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
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

#include "kmetaprops.h"
#include "kpropertiesdialog_p.h"

#include <kfilemetadatawidget.h>
#include <klocale.h>

#include <QtGui/QScrollArea>
#include <QtGui/QVBoxLayout>

using namespace KDEPrivate;

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropertiesDialogPlugin(props),d(new KFileMetaPropsPluginPrivate)
{
    KFileMetaDataWidget* fileMetaDataWidget = new KFileMetaDataWidget();
    fileMetaDataWidget->setItems(properties->items());

    // Embed the FileMetaDataWidget inside a container that has a dummy widget
    // at the bottom. This prevents that the file meta data widget gets vertically stretched.
    QWidget* metaDataWidgetContainer = new QWidget();
    QVBoxLayout* containerLayout = new QVBoxLayout(metaDataWidgetContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    containerLayout->addWidget(fileMetaDataWidget);
    QWidget* stretchWidget = new QWidget(metaDataWidgetContainer);
    stretchWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    containerLayout->addWidget(stretchWidget);

    // The height of FileMetaDataWidget can get very large, so it is embedded
    // into a scrollarea
    QScrollArea* metaDataArea = new QScrollArea();
    metaDataArea->setWidget(metaDataWidgetContainer);
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    properties->addPage(metaDataArea, i18nc("@title:tab", "Information"));
}

KFileMetaPropsPlugin::~KFileMetaPropsPlugin()
{
    delete d;
}

bool KFileMetaPropsPlugin::supports( const KFileItemList& _items )
{
    return true;
}

void KFileMetaPropsPlugin::applyChanges()
{
}

#include "kmetaprops.moc"
