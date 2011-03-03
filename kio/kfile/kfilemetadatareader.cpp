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

#include "kprocess.h"
#include "kstandarddirs.h"

class KFileMetaDataReader::Private
{
public:
    Private(KFileMetaDataReader* parent);
    ~Private();

    void slotLoadingFinished(int exitCode, QProcess::ExitStatus exitStatus);

    bool m_readContextData;
    KProcess* m_process;
    QHash<QString, QVariant> m_metaData;

private:
    KFileMetaDataReader* const q;
};

KFileMetaDataReader::Private::Private(KFileMetaDataReader* parent) :
    m_readContextData(true),
    m_process(new KProcess()),
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

    QString key;
    QVariant value;
    while (!in.atEnd()) {
        in >> key >> value;
        m_metaData.insert(key, value);
    }

    emit q->finished();
}

KFileMetaDataReader::KFileMetaDataReader(const QList<KUrl>& urls, QObject* parent) :
    QObject(parent),
    d(new Private(this))
{
    const QString fileMetaDataReaderExe = KStandardDirs::findExe(QLatin1String("kfilemetadatareader"));
    (*d->m_process) << fileMetaDataReaderExe;

    foreach (const KUrl& url, urls) {
        (*d->m_process) << url.url();
    }

    d->m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    d->m_process->setNextOpenMode(QIODevice::ReadOnly);
    connect(d->m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotLoadingFinished(int, QProcess::ExitStatus)));
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
        if (!d->m_readContextData) {
            (*d->m_process) << "--file";
        }
        d->m_process->start();
    }
}

QHash<QString, QVariant> KFileMetaDataReader::metaData() const
{
    return d->m_metaData;
}

#include "kfilemetadatareader_p.moc"
