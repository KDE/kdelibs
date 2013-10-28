/*****************************************************************************
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kfilemetadataconfigurationwidget.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfilemetainfo.h>
#include <kfilemetainfoitem.h>
#include "knfotranslator_p.h"
#include <klocalizedstring.h>

#include <config-kde4support.h>
#if ! KIO_NO_NEPOMUK
    #define DISABLE_NEPOMUK_LEGACY
    #include <resource.h>
    #include <resourcemanager.h>
    #include <property.h>
    #include <variant.h>

    #include "kfilemetadataprovider_p.h"
#endif

#include <QEvent>
#include <QListWidget>
#include <QVBoxLayout>

class KFileMetaDataConfigurationWidget::Private
{
public:
    Private(KFileMetaDataConfigurationWidget* parent);
    ~Private();

    void init();
    void loadMetaData();
    void addItem(const QUrl& uri);

    /**
     * Is invoked after the meta data model has finished the loading of
     * meta data. The meta data labels will be added to the configuration
     * list.
     */
    void slotLoadingFinished();

    int m_visibleDataTypes;
    KFileItemList m_fileItems;
#if ! KIO_NO_NEPOMUK
    KFileMetaDataProvider* m_provider;
#endif
    QListWidget* m_metaDataList;

private:
    KFileMetaDataConfigurationWidget* const q;
};

KFileMetaDataConfigurationWidget::Private::Private(KFileMetaDataConfigurationWidget* parent) :
    m_visibleDataTypes(0),
    m_fileItems(),
#if ! KIO_NO_NEPOMUK
    m_provider(0),
#endif
    m_metaDataList(0),
    q(parent)
{
    m_metaDataList = new QListWidget(q);
    m_metaDataList->setSelectionMode(QAbstractItemView::NoSelection);
    m_metaDataList->setSortingEnabled(true);

    QVBoxLayout* layout = new QVBoxLayout(q);
    layout->addWidget(m_metaDataList);

#if ! KIO_NO_NEPOMUK
    m_provider = new KFileMetaDataProvider(q);
#endif
}

KFileMetaDataConfigurationWidget::Private::~Private()
{
}

void KFileMetaDataConfigurationWidget::Private::loadMetaData()
{
#if ! KIO_NO_NEPOMUK
    m_provider->setItems(m_fileItems);
    connect(m_provider, SIGNAL(loadingFinished()),
            q, SLOT(slotLoadingFinished()));
#endif
}

void KFileMetaDataConfigurationWidget::Private::addItem(const QUrl& uri)
{
    // Meta information provided by Nepomuk that is already
    // available from KFileItem as "fixed item" (see above)
    // should not be shown as second entry.
    static const char* const hiddenProperties[] = {
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment",         // = fixed item kfileitem#comment
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize",     // = fixed item kfileitem#size
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified",    // = fixed item kfileitem#modified
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent" // hide this property always
        "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType",        // = fixed item kfileitem#type
        "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName",        // hide this property always
        "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",                          // = fixed item kfileitem#type
        0 // mandatory last entry
    };

    int i = 0;
    const QString key = uri.toString();
    while (hiddenProperties[i] != 0) {
        if (key == QLatin1String(hiddenProperties[i])) {
            // the item is hidden
            return;
        }
        ++i;
    }

    // the item is not hidden, add it to the list
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");

#if ! KIO_NO_NEPOMUK
    const QString label = (m_provider == 0)
                          ? KNfoTranslator::instance().translation(uri)
                          : m_provider->label(uri);
#else
    const QString label = KNfoTranslator::instance().translation(uri);
#endif

    QListWidgetItem* item = new QListWidgetItem(label, m_metaDataList);
    item->setData(Qt::UserRole, key);
    const bool show = settings.readEntry(key, true);
    item->setCheckState(show ? Qt::Checked : Qt::Unchecked);
}

void KFileMetaDataConfigurationWidget::Private::slotLoadingFinished()
{
#if ! KIO_NO_NEPOMUK
    // Get all meta information labels that are available for
    // the currently shown file item and add them to the list.
    Q_ASSERT(m_provider != 0);

    const QHash<QUrl, Nepomuk::Variant> data = m_provider->data();
    QHash<QUrl, Nepomuk::Variant>::const_iterator it = data.constBegin();
    while (it != data.constEnd()) {
        addItem(it.key());
        ++it;
    }
#endif
}

KFileMetaDataConfigurationWidget::KFileMetaDataConfigurationWidget(QWidget* parent) :
    QWidget(parent),
    d(new Private(this))
{
}

KFileMetaDataConfigurationWidget::~KFileMetaDataConfigurationWidget()
{
    delete d;
}

void KFileMetaDataConfigurationWidget::setItems(const KFileItemList& items)
{
    d->m_fileItems = items;
}

KFileItemList KFileMetaDataConfigurationWidget::items() const
{
    return d->m_fileItems;
}

void KFileMetaDataConfigurationWidget::save()
{
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup showGroup = config.group("Show");

    const int count = d->m_metaDataList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = d->m_metaDataList->item(i);
        const bool show = (item->checkState() == Qt::Checked);
        const QString key = item->data(Qt::UserRole).toString();
        showGroup.writeEntry(key, show);
    }

    showGroup.sync();
}

bool KFileMetaDataConfigurationWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Polish) {
        // loadMetaData() must be invoked asynchronously, as the list
        // must finish it's initialization first
        QMetaObject::invokeMethod(this, "loadMetaData", Qt::QueuedConnection);
    }
    return QWidget::event(event);;
}

QSize KFileMetaDataConfigurationWidget::sizeHint() const
{
    return d->m_metaDataList->sizeHint();
}


#include "moc_kfilemetadataconfigurationwidget.cpp"
