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

#ifndef KNEWSTUFF2_DXS_SOAP_H
#define KNEWSTUFF2_DXS_SOAP_H

#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

namespace KIO
{
class Job;
}
class KJob;

class QDomDocument;
class QDomNode;
class QDomElement;
class QTcpSocket;

namespace KNS
{

/**
 * KNewStuff transport layer.
 *
 * Both SOAP and cDXS are implemented by this class. Since the latter one is
 * merely a more efficient representation of the former, both are shared in
 * the same class.
 *
 * This class is used internally by the Dxs class.
 *
 * @internal
 */
class Soap : public QObject
{
    Q_OBJECT
public:
    Soap(QObject* parent);
    ~Soap();

    enum Model {
        soap,
        canonicaltree
    };

    /**
     * Set the model to be either soap or canonicaltree
     */
    void setModel(Model m);

    /**
     * Send to the server - uses either soap or tree
     */
    int call(const QDomElement& element, const QString &endpoint);

    /**
     * Name of the QDomElement for node without the namespace
     */
    QString localname(const QDomNode& node);

    /**
     * Find the text element to a xpath like expression
     * @param node xml (eg: \<test\>my_text\</test\>)
     * @param expr an XPath-like expression (eg: "/test")
     * @return the text content of the selected node (eg: "my_text")
     */
    QString xpath(const QDomNode& node, const QString &expr);
    QList<QDomNode> directChildNodes(const QDomNode& node, const QString &name);

signals:
    void signalResult(QDomNode node, int jobid);
    void signalError();

private slots:
    void slotData(KIO::Job *job, const QByteArray& data);
    void slotResult(KJob *job);
    void slotSocket();
    void slotSocketError(QAbstractSocket::SocketError error);

private:
    /**
     * Create the SOAP xml and send it to the server
     * @param element the message
     * @param endpoint server URL
     */
    int call_soap(QDomElement element, const QString& endpoint);

    /**
     * Same as call_soap but with canonicaltree as model
     */
    void call_tree(const QDomElement& element, const QString& endpoint);
    QDomDocument buildtree(QDomDocument doc, QDomElement cur, const QString& data);

    QString m_data;
    Model m_model;
    QTcpSocket *m_socket;
    QByteArray m_buffer;
    bool m_inprogress;
    QMap<KJob*, int> m_jobids;
    int m_lastjobid;
};

}

#endif
