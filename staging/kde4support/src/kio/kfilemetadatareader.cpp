/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
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

#include "kfilemetadatareader_p.h"

#include <QtCore/QProcess>


#include <qstandardpaths.h>

class KFileMetaDataReader::Private
{
public:
    Private(KFileMetaDataReader* parent);
    ~Private();

    void slotLoadingFinished(int exitCode, QProcess::ExitStatus exitStatus);

    bool m_readContextData;
    QProcess* m_process;
    QStringList arguments;
    QHash<QUrl, Nepomuk::Variant> m_metaData;

private:
    KFileMetaDataReader* const q;
};

KFileMetaDataReader::Private::Private(KFileMetaDataReader* parent) :
    m_readContextData(true),
    m_process(new QProcess()),
    m_metaData(),
    q(parent)
{
}

KFileMetaDataReader::Private::~Private()
{
    delete m_process;
}

void KFileMetaDataReader::Private::slotLoadingFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    QDataStream in(QByteArray::fromBase64(m_process->readLine()));

    QUrl key;
    Nepomuk::Variant value;
    while (!in.atEnd()) {
        in >> key;

        // Unlike QVariant no streaming operators are implemented for Nepomuk::Variant.
        // So it is required to manually decode the variant from the stream. See
        // function sendMetaData() in kfilemetadatareaderprocess.cpp for the encoding
        // counterpart.
        int streamType;
        in >> streamType;

        switch (streamType) {
        case 0: {
            QStringList stringList;
            in >> stringList;
            value = stringList;
            break;
        }
        case 1: {
            QString resource;
            in >> resource;
            value = resource;
            break;
        }

        default:
            QVariant variant;
            in >> variant;
            value = Nepomuk::Variant(variant);
        }

        m_metaData.insert(key, value);
    }

    emit q->finished();
}

KFileMetaDataReader::KFileMetaDataReader(const QList<QUrl>& urls, QObject* parent) :
    QObject(parent),
    d(new Private(this))
{
    foreach (const QUrl& url, urls) {
        d->arguments.append(url.toString());
    }

    d->m_process->setReadChannel(QProcess::StandardOutput);
    connect(d->m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotLoadingFinished(int,QProcess::ExitStatus)));
}

KFileMetaDataReader::~KFileMetaDataReader()
{
    delete d;
}

void KFileMetaDataReader::setReadContextData(bool read)
{
    d->m_readContextData = read;
}

bool KFileMetaDataReader::readContextData() const
{
    return d->m_readContextData;
}

void KFileMetaDataReader::start()
{
    if (d->m_process->state() == QProcess::NotRunning) {
        const QString fileMetaDataReaderExe = QStandardPaths::findExecutable(QLatin1String("kfilemetadatareader"));
        if (!d->m_readContextData) {
            d->arguments.append("--file");
        }
        d->m_process->start(fileMetaDataReaderExe, d->arguments, QIODevice::ReadOnly);
    }
}

QHash<QUrl, Nepomuk::Variant> KFileMetaDataReader::metaData() const
{
    return d->m_metaData;
}

#include "moc_kfilemetadatareader_p.cpp"
