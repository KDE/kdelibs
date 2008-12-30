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

#include "dxs.h"

#include "soap.h"

#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"
#include "knewstuff2/core/provider.h"

#include <kdebug.h>

#include <QtXml/qdom.h>
#include <QtCore/QMutableStringListIterator>

using namespace KNS;

Dxs::Dxs(QObject* parent, KNS::Provider * provider)
        : QObject(parent), m_provider(provider)
{
    m_soap = new Soap(this);
    connect(m_soap, SIGNAL(signalResult(QDomNode, int)), SLOT(slotResult(QDomNode, int)));
    connect(m_soap, SIGNAL(signalError()), SLOT(slotError()));
}

Dxs::~Dxs()
{
}

Provider * Dxs::provider()
{
    return m_provider;
}

void Dxs::setEndpoint(KUrl endpoint)
{
    m_endpoint = endpoint;
}

void Dxs::call_info()
{
    QDomDocument doc;
    QDomElement info = doc.createElement("ns:GHNSInfo");
    //QDomText t = doc.createTextNode("text");
    //check.appendChild(t);
    m_soap->call(info, m_endpoint.url());
}

void Dxs::call_categories()
{
    QDomDocument doc;
    QDomElement info = doc.createElement("ns:GHNSCategories");
    m_soap->call(info, m_endpoint.url());
}

void Dxs::call_entries(QString category, QString feed)
{
    //kDebug() << "calling entries on category " << category << " and feed " << feed;
    QDomDocument doc;
    QDomElement entries = doc.createElement("ns:GHNSList");
    QDomElement ecategory = doc.createElement("category");
    QDomText t = doc.createTextNode(category);
    ecategory.appendChild(t);
    entries.appendChild(ecategory);
    if (!feed.isEmpty()) {
        QDomElement efeed = doc.createElement("feed");
        QDomText t2 = doc.createTextNode(feed);
        efeed.appendChild(t2);
        entries.appendChild(efeed);
    }
    int jobid = m_soap->call(entries, m_endpoint.url());
    m_jobfeeds.insert(jobid, m_provider->downloadUrlFeed(feed));
}

void Dxs::call_comments(int id)
{
    //kDebug() << "getting comments for entry: " << id;
    QDomDocument doc;
    QDomElement comments = doc.createElement("ns:GHNSComments");
    QDomElement eid = doc.createElement("id");
    QDomText t = doc.createTextNode(QString::number(id));
    eid.appendChild(t);
    comments.appendChild(eid);
    m_soap->call(comments, m_endpoint.url());
}

void Dxs::call_changes(int id)
{
    QDomDocument doc;
    QDomElement changes = doc.createElement("ns:GHNSChanges");
    QDomElement eid = doc.createElement("id");
    QDomText t = doc.createTextNode(QString::number(id));
    eid.appendChild(t);
    changes.appendChild(eid);
    m_soap->call(changes, m_endpoint.url());
}

void Dxs::call_history(int id)
{
    QDomDocument doc;
    QDomElement history = doc.createElement("ns:GHNSHistory");
    QDomElement eid = doc.createElement("id");
    QDomText t = doc.createTextNode(QString::number(id));
    eid.appendChild(t);
    history.appendChild(eid);
    m_soap->call(history, m_endpoint.url());
}

void Dxs::call_removal(int id)
{
    QDomDocument doc;
    QDomElement removal = doc.createElement("ns:GHNSRemoval");
    QDomElement eid = doc.createElement("id");
    QDomText t = doc.createTextNode(QString::number(id));
    eid.appendChild(t);
    removal.appendChild(eid);
    m_soap->call(removal, m_endpoint.url());
}

void Dxs::call_subscription(int id, bool subscribe)
{
    QDomDocument doc;
    QDomElement subscription = doc.createElement("ns:GHNSSubscription");
    QDomElement eid = doc.createElement("id");
    QDomText t = doc.createTextNode(QString::number(id));
    eid.appendChild(t);
    subscription.appendChild(eid);
    QDomElement esubscribe = doc.createElement("subscribe");
    QDomText t2 = doc.createTextNode((subscribe ? "true" : "false"));
    esubscribe.appendChild(t2);
    subscription.appendChild(esubscribe);
    m_soap->call(subscription, m_endpoint.url());
}

void Dxs::call_comment(int id, QString comment)
{
    //kDebug() << "setting comment: " << comment << " for entry: " << id;
    QDomDocument doc;
    QDomElement ecomment = doc.createElement("ns:GHNSComment");
    QDomElement eid = doc.createElement("id");
    QDomText tid = doc.createTextNode(QString::number(id));
    eid.appendChild(tid);
    ecomment.appendChild(eid);
    QDomElement ecommenttext = doc.createElement("comment");
    QDomText tcomment = doc.createTextNode(comment);
    ecommenttext.appendChild(tcomment);
    ecomment.appendChild(ecommenttext);
    m_soap->call(ecomment, m_endpoint.url());
}

void Dxs::call_rating(int id, int rating)
{
    QDomDocument doc;
    QDomElement erating = doc.createElement("ns:GHNSRating");
    QDomElement eid = doc.createElement("id");
    QDomText tid = doc.createTextNode(QString::number(id));
    eid.appendChild(tid);
    erating.appendChild(eid);
    QDomElement eratingtext = doc.createElement("rating");
    QDomText trating = doc.createTextNode(QString::number(rating));
    eratingtext.appendChild(trating);
    erating.appendChild(eratingtext);
    m_soap->call(erating, m_endpoint.url());
}

void Dxs::slotError()
{
    emit signalError();
}

void Dxs::slotResult(QDomNode node, int jobid)
{
    //kDebug() << "LOCALNAME: " << m_soap->localname(node);

    bool success = true;
    if (m_soap->localname(node) == "Fault") {
        success = false;
        emit signalFault();
        return;
    }

    if (m_soap->localname(node) == "GHNSInfoResponse") {
        QString provider = m_soap->xpath(node, "/provider");
        QString server = m_soap->xpath(node, "/server");
        QString version = m_soap->xpath(node, "/version");

        emit signalInfo(provider, server, version);
    } else if (m_soap->localname(node) == "GHNSCategoriesResponse") {
        QList<KNS::Category*> categories;

        QList<QDomNode> catlist = m_soap->directChildNodes(node, "category");
        for (int i = 0; i < catlist.count(); i++) {
            KNS::Category *category = new KNS::Category();

            QDomNode node = catlist.at(i).toElement();
            QString categoryname = m_soap->xpath(node, "/category");
            QString icon = m_soap->xpath(node, "/icon");
            QString name = m_soap->xpath(node, "/name");
            QString description = m_soap->xpath(node, "/description");

            category->setId(categoryname);
            category->setName(name);
            category->setIcon(icon);
            category->setDescription(description);

            categories << category;
        }

        emit signalCategories(categories);
    } else if (m_soap->localname(node) == "GHNSListResponse") {
        QList<KNS::Entry*> entries;

        Feed * thisFeed = m_jobfeeds.value(jobid);
        QDomNode entriesNode = node.firstChild();
        // FIXME: find a way to put a real assertion in here to ensure the entriesNode is the "entries" node
        //Q_ASSERT(entriesNode.localName() == "entries");

        QList<QDomNode> entrylist = m_soap->directChildNodes(entriesNode, "entry");
        for (int i = 0; i < entrylist.count(); i++) {
            QDomElement element = entrylist.at(i).toElement();
            element.setTagName("stuff");
            KNS::EntryHandler handler(element);
            KNS::Entry *entry = handler.entryptr();

            entries << entry;
            thisFeed->addEntry(entry);
            //kDebug() << "ENTRY: " << entry->name().representation() << " location: " << entry->payload().representation();
        }

        emit signalEntries(entries, thisFeed);
    } else if (m_soap->localname(node) == "GHNSCommentsResponse") {
        QStringList comments;

        QList<QDomNode> comlist = m_soap->directChildNodes(node, "comments");
        for (int i = 0; i < comlist.count(); i++) {
            comments << comlist.at(i).toElement().text();
        }

        emit signalComments(comments);
    } else if (m_soap->localname(node) == "GHNSChangesResponse") {
        QStringList changes;

        QList<QDomNode> changelist = m_soap->directChildNodes(node, "entry");
        for (int i = 0; i < changelist.count(); i++) {
            QDomNode node = changelist.at(i);

            QString version = m_soap->xpath(node, "/version");
            QString changelog = m_soap->xpath(node, "/changelog");
            //kDebug() << "CHANGELOG: " << version << " " << changelog;

            changes << changelog;
        }

        // FIXME: pass (version, changelog) pairs - Python I miss you :-)
        emit signalChanges(changes);
    } else if (m_soap->localname(node) == "GHNSHistoryResponse") {
        QStringList entries;

        QList<QDomNode> entrylist = m_soap->directChildNodes(node, "entry");
        for (int i = 0; i < entrylist.count(); i++) {
            entries << entrylist.at(i).toElement().text();
        }

        emit signalHistory(entries);
    } else if (m_soap->localname(node) == "GHNSRemovalResponse") {
        emit signalRemoval(success);
    } else if (m_soap->localname(node) == "GHNSSubscriptionResponse") {
        emit signalSubscription(success);
    } else if (m_soap->localname(node) == "GHNSCommentResponse") {
        emit signalComment(success);
    } else if (m_soap->localname(node) == "GHNSRatingResponse") {
        emit signalRating(success);
    }
}

#include "dxs.moc"
