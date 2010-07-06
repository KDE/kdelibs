/*****************************************************************************
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

#ifndef KLOADFILEMETADATATHREAD_H
#define KLOADFILEMETADATATHREAD_H

#define DISABLE_NEPOMUK_LEGACY
#include <property.h>
#include <tag.h>
#include <variant.h>

#include <kurl.h>
#include <QList>
#include <QMutex>
#include <QThread>

/**
 * Loads the meta data of files that are
 * required by the widget KMetaDataWidget.
 *
 * Is used in KMetaDataModel.
 */
class KLoadFileMetaDataThread : public QThread
{
    Q_OBJECT

public:
    KLoadFileMetaDataThread();
    virtual ~KLoadFileMetaDataThread();

    /**
     * Starts the thread and loads the meta data for
     * the files given by \p urls. After receiving
     * the signal finished(), the method KLoadFileMetaDataThread::data()
     * provides the loaded meta data.
     */
    void load(const KUrl::List& urls);

    /**
     * Returns the meta data for the URLs given
     * by KLoadFileMetaDataThread::load(). The method only provides
     * valid results after the signal finished() has been
     * emitted.
     */
    QHash<KUrl, Nepomuk::Variant> data() const;

    /**
     * Tells the thread that it should cancel as soon
     * as possible. It is undefined when the thread
     * gets cancelled. The signal finished() will emitted
     * after the cancelling has been done.mergedIt
     */
    void cancel();

    /** @see QThread::run() */
    virtual void run();

Q_SIGNALS:
    /**
     * Is emitted if the thread has finished the loading. KMetaDataModel
     * requires the information which thread has been finished, hence
     * the thread is passed as parameter (using QObject::sender() would
     * be an alternative, but it is less understandable).
     */
    void finished(QThread* thread);

private slots:
    /**
     * Emits the signal finished() including the thread as parameter.
     */
    void slotLoadingFinished();

private:
    /**
     * Temporary helper method until there is a proper formating facility in Nepomuk.
     * Here we simply handle the most common formatting situations that do not look nice
     * when using Nepomuk::Variant::toString().
     */
    QString formatValue(const Nepomuk::Types::Property&, const Nepomuk::Variant& value);

private:
    mutable QMutex m_mutex;
    QHash<KUrl, Nepomuk::Variant> m_data;
    KUrl::List m_urls;

    volatile bool m_canceled;
};
#endif
