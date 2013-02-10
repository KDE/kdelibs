/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
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

#include "kfilemetadataprovider_p.h"

#include <kfileitem.h>
#include <kfilemetadatareader_p.h>
#include "knfotranslator_p.h"
#include <klocale.h>

#include <kurl.h>

#if ! KIO_NO_NEPOMUK
    #define DISABLE_NEPOMUK_LEGACY
    #include "nepomukmassupdatejob.h"
    #include "tagwidget.h"
    #include "tag.h"
    #include "kratingwidget.h"
    #include "resource.h"
    #include "resourcemanager.h"

    #include "kcommentwidget_p.h"
#else
    namespace Nepomuk
    {
        typedef int Tag;
    }
#endif

#include <QEvent>
#include <QLabel>

// Required includes for subDirectoriesCount():
#ifdef Q_OS_WIN
    #include <QDir>
#else
    #include <dirent.h>
    #include <QFile>
#endif

namespace {
    static QString plainText(const QString& richText)
    {
        QString plainText;
        plainText.reserve(richText.length());

        bool skip = false;
        for (int i = 0; i < richText.length(); ++i) {
            const QChar c = richText.at(i);
            if (c == QLatin1Char('<')) {
                skip = true;
            } else if (c == QLatin1Char('>')) {
                skip = false;
            } else if (!skip) {
                plainText.append(c);
            }
        }

        return plainText;
    }
}

// The default size hint of QLabel tries to return a square size.
// This does not work well in combination with layouts that use
// heightForWidth(): In this case it is possible that the content
// of a label might get clipped. By specifying a size hint
// with a maximum width that is necessary to contain the whole text,
// using heightForWidth() assures having a non-clipped text.
class ValueWidget : public QLabel
{
public:
    explicit ValueWidget(QWidget* parent = 0);
    virtual QSize sizeHint() const;
};

ValueWidget::ValueWidget(QWidget* parent) :
    QLabel(parent)
{
}

QSize ValueWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    // TODO: QLabel internally provides already a method sizeForWidth(),
    // that would be sufficient. However this method is not accessible, so
    // as workaround the tags from a richtext are removed manually here to
    // have a proper size hint.
    return metrics.size(Qt::TextSingleLine, plainText(text()));
}



class KFileMetaDataProvider::Private
{

public:
    Private(KFileMetaDataProvider* parent);
    ~Private();

    void slotLoadingFinished();

    void slotRatingChanged(unsigned int rating);
    void slotTagsChanged(const QList<Nepomuk::Tag>& tags);
    void slotCommentChanged(const QString& comment);

    void slotMetaDataUpdateDone();
    void slotTagClicked(const Nepomuk::Tag& tag);
    void slotLinkActivated(const QString& link);

    /**
     * Disables the metadata widget and starts the job that
     * changes the meta data asynchronously. After the job
     * has been finished, the metadata widget gets enabled again.
     */
    void startChangeDataJob(KJob* job);

#if ! KIO_NO_NEPOMUK
    QList<Nepomuk::Resource> resourceList() const;
    QWidget* createRatingWidget(int rating, QWidget* parent);
    QWidget* createTagWidget(const QList<Nepomuk::Tag>& tags, QWidget* parent);
    QWidget* createCommentWidget(const QString& comment, QWidget* parent);
#endif
    QWidget* createValueWidget(const QString& value, QWidget* parent);

    /*
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    bool m_readOnly;
    bool m_nepomukActivated;
    QList<KFileItem> m_fileItems;

#if ! KIO_NO_NEPOMUK
    QHash<QUrl, Nepomuk::Variant> m_data;

    QList<KFileMetaDataReader*> m_metaDataReaders;
    KFileMetaDataReader* m_latestMetaDataReader;

    QPointer<KRatingWidget> m_ratingWidget;
    QPointer<Nepomuk::TagWidget> m_tagWidget;
    QPointer<KCommentWidget> m_commentWidget;
#endif

private:
    KFileMetaDataProvider* const q;
};

KFileMetaDataProvider::Private::Private(KFileMetaDataProvider* parent) :
    m_readOnly(false),
    m_nepomukActivated(false),
    m_fileItems(),
#if ! KIO_NO_NEPOMUK
    m_data(),
    m_metaDataReaders(),
    m_latestMetaDataReader(0),
    m_ratingWidget(),
    m_tagWidget(),
    m_commentWidget(),
#endif
    q(parent)
{
#if ! KIO_NO_NEPOMUK
    m_nepomukActivated = Nepomuk::ResourceManager::instance()->initialized();
#endif
}

KFileMetaDataProvider::Private::~Private()
{
#if ! KIO_NO_NEPOMUK
    qDeleteAll(m_metaDataReaders);
#endif
}

void KFileMetaDataProvider::Private::slotLoadingFinished()
{
#if ! KIO_NO_NEPOMUK
    KFileMetaDataReader* finishedMetaDataReader = qobject_cast<KFileMetaDataReader*>(q->sender());
    // The process that has emitted the finished() signal
    // will get deleted and removed from m_metaDataReaders.
    for (int i = 0; i < m_metaDataReaders.count(); ++i) {
        KFileMetaDataReader* metaDataReader = m_metaDataReaders[i];
        if (metaDataReader == finishedMetaDataReader) {
            m_metaDataReaders.removeAt(i);
            if (metaDataReader != m_latestMetaDataReader) {
                // Ignore data of older processs, as the data got
                // obsolete by m_latestMetaDataReader.
                metaDataReader->deleteLater();
                return;
            }
        }
    }

    m_data = m_latestMetaDataReader->metaData();
    m_latestMetaDataReader->deleteLater();

    if (m_fileItems.count() == 1) {
        // TODO: Handle case if remote URLs are used properly. isDir() does
        // not work, the modification date needs also to be adjusted...
        const KFileItem& item = m_fileItems.first();

        if (item.isDir()) {
            const int count = subDirectoriesCount(item.url().pathOrUrl());
            if (count == -1) {
                m_data.insert(QUrl("kfileitem#size"), QString("Unknown"));
            } else {
                const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                m_data.insert(QUrl("kfileitem#size"), itemCountString);
            }
        } else {
            m_data.insert(QUrl("kfileitem#size"), KIO::convertSize(item.size()));
        }
        m_data.insert(QUrl("kfileitem#type"), item.mimeComment());
        m_data.insert(QUrl("kfileitem#modified"), KLocale::global()->formatDateTime(item.time(KFileItem::ModificationTime), KLocale::FancyLongDate));
        m_data.insert(QUrl("kfileitem#owner"), item.user());
        m_data.insert(QUrl("kfileitem#permissions"), item.permissionsString());
    } else if (m_fileItems.count() > 1) {
        // Calculate the size of all items
        quint64 totalSize = 0;
        foreach (const KFileItem& item, m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.insert(QUrl("kfileitem#totalSize"), KIO::convertSize(totalSize));
    }
#endif

    emit q->loadingFinished();
}

void KFileMetaDataProvider::Private::slotRatingChanged(unsigned int rating)
{
#if ! KIO_NO_NEPOMUK
    Nepomuk::MassUpdateJob* job = Nepomuk::MassUpdateJob::rateResources(resourceList(), rating);
    startChangeDataJob(job);
#else
    Q_UNUSED(rating);
#endif
}

void KFileMetaDataProvider::Private::slotTagsChanged(const QList<Nepomuk::Tag>& tags)
{
#if ! KIO_NO_NEPOMUK
    if (!m_tagWidget.isNull()) {
        m_tagWidget.data()->setSelectedTags(tags);

        Nepomuk::MassUpdateJob* job = Nepomuk::MassUpdateJob::tagResources(resourceList(), tags);
        startChangeDataJob(job);
    }
#else
    Q_UNUSED(tags);
#endif
}

void KFileMetaDataProvider::Private::slotCommentChanged(const QString& comment)
{
#if ! KIO_NO_NEPOMUK
    Nepomuk::MassUpdateJob* job = Nepomuk::MassUpdateJob::commentResources(resourceList(), comment);
    startChangeDataJob(job);
#else
    Q_UNUSED(comment);
#endif
}

void KFileMetaDataProvider::Private::slotTagClicked(const Nepomuk::Tag& tag)
{
#if ! KIO_NO_NEPOMUK
    emit q->urlActivated(tag.resourceUri());
#else
    Q_UNUSED(tag);
#endif
}

void KFileMetaDataProvider::Private::slotLinkActivated(const QString& link)
{
    emit q->urlActivated(QUrl(link));
}

void KFileMetaDataProvider::Private::startChangeDataJob(KJob* job)
{
    connect(job, SIGNAL(result(KJob*)),
            q, SIGNAL(dataChangeFinished()));
    emit q->dataChangeStarted();
    job->start();
}

#if ! KIO_NO_NEPOMUK
QList<Nepomuk::Resource> KFileMetaDataProvider::Private::resourceList() const
{
    QList<Nepomuk::Resource> list;
    foreach (const KFileItem& item, m_fileItems) {
        const QUrl url = item.nepomukUri();
        if(url.isValid())
            list.append(Nepomuk::Resource(url));
    }
    return list;
}

QWidget* KFileMetaDataProvider::Private::createRatingWidget(int rating, QWidget* parent)
{
    KRatingWidget* ratingWidget = new KRatingWidget(parent);
    const Qt::Alignment align = (ratingWidget->layoutDirection() == Qt::LeftToRight) ?
                                Qt::AlignLeft : Qt::AlignRight;
    ratingWidget->setAlignment(align);
    ratingWidget->setRating(rating);
    const QFontMetrics metrics(parent->font());
    ratingWidget->setPixmapSize(metrics.height());

    connect(ratingWidget, SIGNAL(ratingChanged(uint)),
            q, SLOT(slotRatingChanged(uint)));

    m_ratingWidget = ratingWidget;

    return ratingWidget;
}

QWidget* KFileMetaDataProvider::Private::createTagWidget(const QList<Nepomuk::Tag>& tags, QWidget* parent)
{
    Nepomuk::TagWidget* tagWidget = new Nepomuk::TagWidget(parent);
    tagWidget->setModeFlags(m_readOnly
                            ? Nepomuk::TagWidget::MiniMode | Nepomuk::TagWidget::ReadOnly
                            : Nepomuk::TagWidget::MiniMode);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, SIGNAL(selectionChanged(QList<Nepomuk::Tag>)),
            q, SLOT(slotTagsChanged(QList<Nepomuk::Tag>)));
    connect(tagWidget, SIGNAL(tagClicked(Nepomuk::Tag)),
            q, SLOT(slotTagClicked(Nepomuk::Tag)));

    m_tagWidget = tagWidget;

    return tagWidget;
}

QWidget* KFileMetaDataProvider::Private::createCommentWidget(const QString& comment, QWidget* parent)
{
    KCommentWidget* commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, SIGNAL(commentChanged(QString)),
            q, SLOT(slotCommentChanged(QString)));

    m_commentWidget = commentWidget;

    return commentWidget;
}
#endif

QWidget* KFileMetaDataProvider::Private::createValueWidget(const QString& value, QWidget* parent)
{
    ValueWidget* valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    valueWidget->setText(m_readOnly ? plainText(value) : value);
    connect(valueWidget, SIGNAL(linkActivated(QString)), q, SLOT(slotLinkActivated(QString)));
    return valueWidget;
}

KFileMetaDataProvider::KFileMetaDataProvider(QObject* parent) :
    QObject(parent),
    d(new Private(this))
{
}

KFileMetaDataProvider::~KFileMetaDataProvider()
{
    delete d;
}

void KFileMetaDataProvider::setItems(const KFileItemList& items)
{
    d->m_fileItems = items;

#if ! KIO_NO_NEPOMUK
    if (items.isEmpty()) {
        return;
    }
    Q_PRIVATE_SLOT(d, void slotDataChangeStarted())
    Q_PRIVATE_SLOT(d, void slotDataChangeFinished())
    QList<QUrl> urls;
    foreach (const KFileItem& item, items) {
        const QUrl url = item.nepomukUri();
        if (url.isValid()) {
            urls.append(url);
        }
    }

    d->m_latestMetaDataReader = new KFileMetaDataReader(urls);
    d->m_latestMetaDataReader->setReadContextData(d->m_nepomukActivated);
    connect(d->m_latestMetaDataReader, SIGNAL(finished()), this, SLOT(slotLoadingFinished()));
    d->m_metaDataReaders.append(d->m_latestMetaDataReader);
    d->m_latestMetaDataReader->start();
#endif
}

QString KFileMetaDataProvider::label(const QUrl& metaDataUri) const
{
    struct TranslationItem {
        const char* const key;
        const char* const context;
        const char* const value;
    };

    static const TranslationItem translations[] = {
        { "kfileitem#comment", I18N_NOOP2_NOSTRIP("@label", "Comment") },
        { "kfileitem#modified", I18N_NOOP2_NOSTRIP("@label", "Modified") },
        { "kfileitem#owner", I18N_NOOP2_NOSTRIP("@label", "Owner") },
        { "kfileitem#permissions", I18N_NOOP2_NOSTRIP("@label", "Permissions") },
        { "kfileitem#rating", I18N_NOOP2_NOSTRIP("@label", "Rating") },
        { "kfileitem#size", I18N_NOOP2_NOSTRIP("@label", "Size") },
        { "kfileitem#tags", I18N_NOOP2_NOSTRIP("@label", "Tags") },
        { "kfileitem#totalSize", I18N_NOOP2_NOSTRIP("@label", "Total Size") },
        { "kfileitem#type", I18N_NOOP2_NOSTRIP("@label", "Type") },
        { 0, 0, 0} // Mandatory last entry
    };

    static QHash<QString, QString> hash;
    if (hash.isEmpty()) {
        const TranslationItem* item = &translations[0];
        while (item->key != 0) {
            hash.insert(item->key, i18nc(item->context, item->value));
            ++item;
        }
    }

    QString value = hash.value(metaDataUri.url());
    if (value.isEmpty()) {
        value = KNfoTranslator::instance().translation(metaDataUri);
    }

    return value;
}

QString KFileMetaDataProvider::group(const QUrl& metaDataUri) const
{
    QString group; // return value

    const QString uri = metaDataUri.url();
    if (uri == QLatin1String("kfileitem#type")) {
        group = QLatin1String("0FileItemA");
    } else if (uri == QLatin1String("kfileitem#size")) {
        group = QLatin1String("0FileItemB");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width")) {
        group = QLatin1String("0SizeA");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height")) {
        group = QLatin1String("0SizeB");
    }

    return group;
}

KFileItemList KFileMetaDataProvider::items() const
{
    return d->m_fileItems;
}

void KFileMetaDataProvider::setReadOnly(bool readOnly)
{
    d->m_readOnly = readOnly;
}

bool KFileMetaDataProvider::isReadOnly() const
{
    return d->m_readOnly;
}

#if ! KIO_NO_NEPOMUK
QHash<QUrl, Nepomuk::Variant> KFileMetaDataProvider::data() const
{
    return d->m_data;
}

QWidget* KFileMetaDataProvider::createValueWidget(const QUrl& metaDataUri,
                                                  const Nepomuk::Variant& value,
                                                  QWidget* parent) const
{
    Q_ASSERT(parent != 0);
    QWidget* widget = 0;

    if (d->m_nepomukActivated) {
        const QString uri = metaDataUri.url();
        if (uri == QLatin1String("kfileitem#rating")) {
            widget = d->createRatingWidget(value.toInt(), parent);
        } else if (uri == QLatin1String("kfileitem#tags")) {
            const QStringList tagNames = value.toStringList();
            QList<Nepomuk::Tag> tags;
            foreach (const QString& tagName, tagNames) {
                tags.append(Nepomuk::Tag(tagName));
            }

            widget = d->createTagWidget(tags, parent);
        } else if (uri == QLatin1String("kfileitem#comment")) {
            widget = d->createCommentWidget(value.toString(), parent);
        }
    }

    if (widget == 0) {
        widget = d->createValueWidget(value.toString(), parent);
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());

    return widget;
}
#endif

int KFileMetaDataProvider::Private::subDirectoriesCount(const QString& path)
{
#ifdef Q_OS_WIN
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#else
    // Taken from kdelibs/kio/kio/kdirmodel.cpp
    // Copyright (C) 2006 David Faure <faure@kde.org>

    int count = -1;
    DIR* dir = ::opendir(QFile::encodeName(path));
    if (dir) {
        count = 0;
        struct dirent *dirEntry = 0;
        while ((dirEntry = ::readdir(dir))) { // krazy:exclude=syscalls
            if (dirEntry->d_name[0] == '.') {
                if (dirEntry->d_name[1] == '\0') {
                    // Skip "."
                    continue;
                }
                if (dirEntry->d_name[1] == '.' && dirEntry->d_name[2] == '\0') {
                    // Skip ".."
                    continue;
                }
            }
            ++count;
        }
        ::closedir(dir);
    }
    return count;
#endif
}

#include "moc_kfilemetadataprovider_p.cpp"
