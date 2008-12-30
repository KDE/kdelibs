/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF2_DXS_DXS_H
#define KNEWSTUFF2_DXS_DXS_H

#include <knewstuff2/core/entry.h>
#include <QtCore/QObject>

#include <kurl.h>

class QDomNode;

namespace KNS
{

class Soap;
class Category;
class Feed;
class Provider;

/**
 * KNewStuff DXS proxy.
 *
 * This class serves as a direct proxy for all of the web service calls that
 * are offered by servers implementing the DXS specification.
 * The DXS proxy will use either SOAP or cDXS to communicate with the service.
 *
 * This class is used internally by the DxsEngine class.
 *
 * @internal
 */
class Dxs : public QObject
{
    Q_OBJECT
public:
    Dxs(QObject* parent, KNS::Provider * provider);
    ~Dxs();
    void setEndpoint(KUrl endpoint);

    /**
     *
     */
    void call_info();

    /**
     *
     */
    void call_categories();

    /**
     *
     * @param category
     * @param feed
     */
    void call_entries(QString category, QString feed);

    /**
     *
     * @param id
     */
    void call_comments(int id);

    /**
     *
     * @param id
     */
    void call_changes(int id);

    /**
     *
     * @param id
     */
    void call_history(int id);

    /**
     *
     * @param id
     */
    void call_removal(int id);

    /**
     *
     * @param id
     * @param subscribe
     */
    void call_subscription(int id, bool subscribe);

    /**
     *
     * @param id
     * @param comment
     */
    void call_comment(int id, QString comment);

    /**
     *        Change the rating
     * @param id
     * @param rating
     */
    void call_rating(int id, int rating);

    Provider *provider();

signals:
    void signalInfo(QString provider, QString server, QString version);
    void signalCategories(QList<KNS::Category*> categories);
    void signalEntries(KNS::Entry::List entries, Feed * feed);
    void signalComments(QStringList comments);
    void signalChanges(QStringList comments);
    void signalHistory(QStringList entries);
    void signalRemoval(bool success);
    void signalSubscription(bool success);
    void signalComment(bool success);
    void signalRating(bool success);
    void signalFault();
    void signalError();

private slots:
    /**
     * Valid response from server - parsed here to emit the corresponding signal
     * @param node
     */
    void slotResult(QDomNode node, int jobid);

    /**
     *        Communication error
     */
    void slotError();

private:
    Soap *m_soap;
    KUrl m_endpoint;
    Provider *m_provider;
    QMap<int, Feed*> m_jobfeeds;
};

}

#endif
