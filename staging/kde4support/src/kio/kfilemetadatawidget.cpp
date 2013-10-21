/*****************************************************************************
 * Copyright (C) 2008-2010 by Sebastian Trueg <trueg@kde.org>                *
 * Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>                 *
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

#include "kfilemetadatawidget.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfileitem.h>
#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>

#include <config-kde4support.h>
#if ! KIO_NO_NEPOMUK
    #define DISABLE_NEPOMUK_LEGACY

    #include <property.h>
    #include <tag.h>

    #include <QSpacerItem>

    #include "kfilemetadataprovider_p.h"
#endif

class KFileMetaDataWidget::Private
{
public:
    struct Row
    {
        QLabel* label;
        QWidget* value;
    };

    Private(KFileMetaDataWidget* parent);
    ~Private();

    /**
     * Initializes the configuration file "kmetainformationrc"
     * with proper default settings for the first start in
     * an uninitialized environment.
     */
    void initMetaInfoSettings();

    /**
     * Parses the configuration file "kmetainformationrc" and
     * updates the visibility of all rows that got their data
     * from KFileItem.
     */
    void updateFileItemRowsVisibility();

    void deleteRows();

    void slotLoadingFinished();
    void slotLinkActivated(const QString& link);
    void slotDataChangeStarted();
    void slotDataChangeFinished();

#if ! KIO_NO_NEPOMUK
    QList<QUrl> sortedKeys(const QHash<QUrl, Nepomuk::Variant>& data) const;

    /**
     * @return True, if at least one of the file items \a m_fileItems has
     *         a valid Nepomuk URI.
     */
    bool hasNepomukUris() const;
#endif

    QList<Row> m_rows;
#if ! KIO_NO_NEPOMUK
    KFileMetaDataProvider* m_provider;
#endif
    QGridLayout* m_gridLayout;

private:
    KFileMetaDataWidget* const q;
};

KFileMetaDataWidget::Private::Private(KFileMetaDataWidget* parent) :
    m_rows(),
#if ! KIO_NO_NEPOMUK
    m_provider(0),
#endif
    m_gridLayout(0),
    q(parent)
{
    initMetaInfoSettings();

#if ! KIO_NO_NEPOMUK
    // TODO: If KFileMetaDataProvider might get a public class in future KDE releases,
    // the following code should be moved into KFileMetaDataWidget::setModel():
    m_provider = new KFileMetaDataProvider(q);
    connect(m_provider, SIGNAL(loadingFinished()), q, SLOT(slotLoadingFinished()));
    connect(m_provider, SIGNAL(urlActivated(QUrl)), q, SIGNAL(urlActivated(QUrl)));
#endif
}

KFileMetaDataWidget::Private::~Private()
{
}

void KFileMetaDataWidget::Private::initMetaInfoSettings()
{
    const int currentVersion = 3; // increase version, if the blacklist of disabled
                                  // properties should be updated

    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    if (config.group("Misc").readEntry("version", 0) < currentVersion) {
        // The resource file is read the first time. Assure
        // that some meta information is disabled per default.

        // clear old info
        config.deleteGroup("Show");
        KConfigGroup settings = config.group("Show");

        static const char* const disabledProperties[] = {
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#depends",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#description",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating",
            "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
            "kfileitem#owner",
            "kfileitem#permissions",
            0 // mandatory last entry
        };

        for (int i = 0; disabledProperties[i] != 0; ++i) {
            settings.writeEntry(disabledProperties[i], false);
        }

        // mark the group as initialized
        config.group("Misc").writeEntry("version", currentVersion);
    }
}

void KFileMetaDataWidget::Private::deleteRows()
{
    foreach (const Row& row, m_rows) {
        delete row.label;
        delete row.value;
    }
    m_rows.clear();
}

void KFileMetaDataWidget::Private::slotLoadingFinished()
{
#if ! KIO_NO_NEPOMUK
    deleteRows();

    if (!hasNepomukUris()) {
        q->updateGeometry();
        emit q->metaDataRequestFinished(m_provider->items());
        return;
    }

    if (m_gridLayout == 0) {
        m_gridLayout = new QGridLayout(q);
        m_gridLayout->setMargin(0);
        m_gridLayout->setSpacing(q->fontMetrics().height() / 4);
    }

    QHash<QUrl, Nepomuk::Variant> data = m_provider->data();

    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    QHash<QUrl, Nepomuk::Variant>::iterator it = data.begin();
    while (it != data.end()) {
        const QString uriString = it.key().url();
        if (!settings.readEntry(uriString, true) ||
            !Nepomuk::Types::Property(it.key()).userVisible()) {
            it = data.erase(it);
        } else {
            ++it;
        }
    }

    // Iterate through all remaining items embed the label
    // and the value as new row in the widget
    int rowIndex = 0;
    const QList<QUrl> keys = sortedKeys(data);
    foreach (const QUrl& key, keys) {
        const Nepomuk::Variant value = data[key];
        QString itemLabel = m_provider->label(key);
        itemLabel.append(QLatin1Char(':'));

        // Create label
        QLabel* label = new QLabel(itemLabel, q);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setForegroundRole(q->foregroundRole());
        label->setFont(q->font());
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignTop | Qt::AlignRight);

        // Create value-widget
        QWidget* valueWidget = m_provider->createValueWidget(key, value, q);

        // Add the label and value-widget to grid layout
        m_gridLayout->addWidget(label, rowIndex, 0, Qt::AlignRight);
        const int spacerWidth = QFontMetrics(q->font()).size(Qt::TextSingleLine, " ").width();
        m_gridLayout->addItem(new QSpacerItem(spacerWidth, 1), rowIndex, 1);
        m_gridLayout->addWidget(valueWidget, rowIndex, 2, Qt::AlignLeft);

        // Remember the label and value-widget as row
        Row row;
        row.label = label;
        row.value = valueWidget;
        m_rows.append(row);

        ++rowIndex;
    }
#endif

    q->updateGeometry();
#if ! KIO_NO_NEPOMUK
    emit q->metaDataRequestFinished(m_provider->items());
#endif
}

void KFileMetaDataWidget::Private::slotLinkActivated(const QString& link)
{
    const QUrl url(link);
    if (url.isValid()) {
        emit q->urlActivated(url);
    }
}

void KFileMetaDataWidget::Private::slotDataChangeStarted()
{
    q->setEnabled(false);
}

void KFileMetaDataWidget::Private::slotDataChangeFinished()
{
    q->setEnabled(true);
}

#if ! KIO_NO_NEPOMUK
QList<QUrl> KFileMetaDataWidget::Private::sortedKeys(const QHash<QUrl, Nepomuk::Variant>& data) const
{
    // Create a map, where the translated label prefixed with the
    // sort priority acts as key. The data of each entry is the URI
    // of the data. By this the all URIs are sorted by the sort priority
    // and sub sorted by the translated labels.
    QMap<QString, QUrl> map;
    QHash<QUrl, Nepomuk::Variant>::const_iterator hashIt = data.constBegin();
    while (hashIt != data.constEnd()) {
        const QUrl uri = hashIt.key();

        QString key = m_provider->group(uri);
        key += m_provider->label(uri);

        map.insert(key, uri);
        ++hashIt;
    }

    // Apply the URIs from the map to the list that will get returned.
    // The list will then be alphabetically ordered by the translated labels of the URIs.
    QList<QUrl> list;
    QMap<QString, QUrl>::const_iterator mapIt = map.constBegin();
    while (mapIt != map.constEnd()) {
        list.append(mapIt.value());
        ++mapIt;
    }

    return list;
}

bool KFileMetaDataWidget::Private::hasNepomukUris() const
{
    foreach (const KFileItem& fileItem, m_provider->items()) {
        if (fileItem.nepomukUri().isValid()) {
            return true;
        }
    }
    return false;
}
#endif

KFileMetaDataWidget::KFileMetaDataWidget(QWidget* parent) :
    QWidget(parent),
    d(new Private(this))
{
}

KFileMetaDataWidget::~KFileMetaDataWidget()
{
    delete d;
}

void KFileMetaDataWidget::setItems(const KFileItemList& items)
{
#if KIO_NO_NEPOMUK
    Q_UNUSED(items)
#else
    d->m_provider->setItems(items);
#endif
}

KFileItemList KFileMetaDataWidget::items() const
{
#if ! KIO_NO_NEPOMUK
    return d->m_provider->items();
#else
    return KFileItemList();
#endif
}

void KFileMetaDataWidget::setReadOnly(bool readOnly)
{
#if KIO_NO_NEPOMUK
    Q_UNUSED(readOnly)
#else
    d->m_provider->setReadOnly(readOnly);
#endif
}

bool KFileMetaDataWidget::isReadOnly() const
{
#if ! KIO_NO_NEPOMUK
    return d->m_provider->isReadOnly();
#else
    return true;
#endif
}

QSize KFileMetaDataWidget::sizeHint() const
{
    if (d->m_gridLayout == 0) {
        return QWidget::sizeHint();
    }

    // Calculate the required width for the labels and values
    int leftWidthMax = 0;
    int rightWidthMax = 0;
    int rightWidthAverage = 0;
    foreach (const Private::Row& row, d->m_rows) {
        const QWidget* valueWidget = row.value;
        const int rightWidth = valueWidget->sizeHint().width();
        rightWidthAverage += rightWidth;
        if (rightWidth > rightWidthMax) {
            rightWidthMax = rightWidth;
        }

        const int leftWidth = row.label->sizeHint().width();
        if (leftWidth > leftWidthMax) {
            leftWidthMax = leftWidth;
        }
    }

    // Some value widgets might return a very huge width for the size hint.
    // Limit the maximum width to the double width of the overall average
    // to assure a less messed layout.
    if (d->m_rows.count() > 1) {
        rightWidthAverage /= d->m_rows.count();
        if (rightWidthMax > rightWidthAverage * 2) {
            rightWidthMax = rightWidthAverage * 2;
        }
    }

    // Based on the available width calculate the required height
    int height = d->m_gridLayout->margin() * 2 + d->m_gridLayout->spacing() * (d->m_rows.count() - 1);
    foreach (const Private::Row& row, d->m_rows) {
        const QWidget* valueWidget = row.value;
        const int rowHeight = qMax(row.label->heightForWidth(leftWidthMax),
                                   valueWidget->heightForWidth(rightWidthMax));
        height += rowHeight;
    }

    const int width = d->m_gridLayout->margin() * 2 + leftWidthMax +
                      d->m_gridLayout->spacing() + rightWidthMax;

    return QSize(width, height);
}

bool KFileMetaDataWidget::event(QEvent* event)
{
    return QWidget::event(event);
}

#include "moc_kfilemetadatawidget.cpp"
