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

#include "soap.h"

#include <QtXml/qdom.h>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QRegExp>

#include <kio/job.h>
#include <kurl.h>
#include <kdebug.h>

#include <string.h> // memcpy()

using namespace KNS;

Soap::Soap(QObject* parent)
        : QObject(parent)
{
    //m_model = canonicaltree;
    m_model = soap;
    m_socket = NULL;
    m_lastjobid = 0;
    //m_inprogress = false;
}

Soap::~Soap()
{
}

int Soap::call(const QDomElement& element, const QString &endpoint)
{
    //if(m_inprogress)
    //{
    //    kWarning() << "Discarding SOAP/CTS call!";
    //    return;
    //}

    if (m_model == canonicaltree) {
        call_tree(element, endpoint);
        // cDxs doesn't support job ids yet
        return -1;
    } else {
        return call_soap(element, endpoint);
    }

    //m_inprogress = true;
}

void Soap::call_tree(const QDomElement& element, const QString &endpoint)
{
    QString s;

    s += localname(element);
    s += '(';
    QDomNodeList l = element.childNodes();
    for (int i = 0; i < l.count(); i++) {
        QDomNode tmp = l.item(i);
        s += localname(tmp);
        s += '(';
        s += xpath(tmp, "/");
        s += ')';
        s += '\n';
    }
    s += ")\n";

    //kDebug() << "<CanonicalTree>" << s;

    QByteArray data = s.toUtf8();

    //kDebug() << "Call(socket)!";

    KUrl url(endpoint);
    QString hostname = url.host();
    int port = 30303/*url.port()*/;

    m_socket = new QTcpSocket();
    m_socket->connectToHost(hostname, port);

    m_socket->write(data, data.size());

    connect(m_socket,
            SIGNAL(readyRead()),
            SLOT(slotSocket()));
    connect(m_socket,
            SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(slotSocketError(QAbstractSocket::SocketError)));

    m_buffer = QByteArray();
}

int Soap::call_soap(QDomElement element, const QString &endpoint)
{
    //kDebug() << "calling soap";
    KUrl url(endpoint);

    QDomDocument doc;
    QDomElement env = doc.createElement("SOAP-ENV:Envelope");
    env.setAttribute("xmlns:SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/");
    doc.appendChild(env);
    QDomElement body = doc.createElement("SOAP-ENV:Body");
    env.appendChild(body);
    element.setAttribute("xmlns:ns", "urn:DXS");
    body.appendChild(element);

    QString s = doc.toString();
    QByteArray data = s.toUtf8();

    //kDebug() << "HTTP-POST " << url.prettyUrl();
    //kDebug() << "HTTP-POST " << s;

    KIO::TransferJob *job;
    job = KIO::http_post(url, data, KIO::HideProgressInfo);
    int thisjobid = ++m_lastjobid;
    m_jobids.insert(job, thisjobid);
    job->addMetaData("content-type", "Content-Type: text/xml");

    //kDebug() << "Call!";

    connect(job,
            SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(job,
            SIGNAL(result(KJob*)),
            SLOT(slotResult(KJob*)));

    m_buffer = QByteArray();
    return thisjobid;
}

void Soap::slotData(KIO::Job *job, const QByteArray& data)
{
    Q_UNUSED(job);

    //kDebug() << "Length(before): " << m_buffer.size();
    //kDebug() << "Append-Length: " << data.size();

    int bufferlen = m_buffer.size();
    m_buffer.resize(bufferlen + data.size());

    //m_buffer.append(data);
    // FIXME: No memcpy() in Qt??? Really, no qMemCopy()? :-)
    memcpy(m_buffer.data() + bufferlen, data.data(), data.size());

    //kDebug() << "Length(after): " << m_buffer.size();
}

void Soap::slotResult(KJob *job)
{
    //kDebug() << "Result!";

    if ((job) && (job->error())) {
        //job->showErrorDialog(this);
        //kDebug() << "SOAP ERROR!";
        emit signalError();
        return;
    }

    //kDebug() << "CSTRING: '" << m_buffer << "'";

    int bufferlen = m_buffer.size();
    m_buffer.resize(bufferlen + 1);
    m_buffer.data()[bufferlen] = 0;
    m_data = QString::fromUtf8(m_buffer);

    //kDebug() << "STRING: '" << m_data << "'";

    if (m_model == soap) {
        QDomDocument doc;
        doc.setContent(m_data);

        QDomElement envelope = doc.documentElement();
        QDomNode bodynode = envelope.firstChild();
        QDomNode contentnode = bodynode.firstChild();

        //kDebug() << "(signal) Result!";

        //m_inprogress = false;
        emit signalResult(contentnode, m_jobids.value(job));
        m_jobids.remove(job);
    } else {
        QDomDocument doc;

        // FIXME: dummy data
        //m_data = QString("GHNSRemovalResponse(ok(true)\nauthorative(true))");
        //kDebug() << m_data;

        m_data = m_data.simplified();
        doc = buildtree(doc, doc.documentElement(), m_data);

        QDomElement root = doc.documentElement();

        //kDebug() << "(signal) Result!";
        //kDebug() << doc.toString();

        //m_inprogress = false;
        emit signalResult(root, -1);
    }
}

QString Soap::localname(const QDomNode& node)
{
    QDomElement el = node.toElement();
    QString s = el.tagName().section(':', -1);
    return s;
}

QList<QDomNode> Soap::directChildNodes(const QDomNode& node, const QString &name)
{
    QList<QDomNode> list;
    QDomNode n = node.firstChild();
    while (!n.isNull()) {
        if ((n.isElement()) && (n.toElement().tagName() == name)) {
            list.append(n);
        }

        n = n.nextSibling();
    }
    return list;
}

QString Soap::xpath(const QDomNode& node, const QString &expr)
{
//  if(m_model == canonicaltree)
//  {
//      //QString provider = m_soap->xpath(node, "/SOAP-ENC:Array/provider");
//      expr = expr.section("/", 2);
//      // FIXME: Array handling for Canonical Tree Structures?
//      kDebug() << "EXPR " << expr;
//  }

    QDomNode n = node;
    const QStringList explist = expr.split('/', QString::SkipEmptyParts);
    for (QStringList::const_iterator it = explist.begin(); it != explist.end(); ++it) {
        QDomElement el = n.toElement();
        QDomNodeList l = el.elementsByTagName((*it));
        if (!l.size()) {
            return QString();
        }
        n = l.item(0);
    }
    QString s = n.toElement().text();
    return s;
}

void Soap::setModel(Model m)
{
    m_model = m;
}

void Soap::slotSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    //kDebug() << "socket: error";

    delete m_socket;
    m_socket = NULL;

    //m_inprogress = false;
}

void Soap::slotSocket()
{
    //kDebug() << "socket: data";

    QByteArray a;
    a.resize(m_socket->bytesAvailable());
    m_socket->read(a.data(), m_socket->bytesAvailable());

    //kDebug() << "DATA" << a;

    slotData(NULL, a);

    if (m_socket->atEnd()) {
        m_socket->close();
//      delete m_socket;
        m_socket = NULL;

        slotResult(NULL);
    }
}

QDomDocument Soap::buildtree(QDomDocument doc, QDomElement cur, const QString& data)
{
    int start = -1, end = -1;
    int offset = 0;
    int stack = 0;
    bool quoted = false;

    if (data.indexOf('(') == -1) {
        QDomText t = doc.createTextNode(data);
        cur.appendChild(t);
        return doc;
    }

    for (int i = 0; i < data.length(); i++) {
        const QChar c = data.at(i);
        if (quoted) {
            quoted = false;
            continue;
        }
        if (c == '\\') {
            quoted = true;
        } else if (c == '(') {
            stack++;
            if (start == -1) {
                start = i;
            }
        } else if (c == ')') {
            stack--;
            if ((stack == 0) && (end == -1)) {
                end = i;
                //kDebug() << "START-END: " << start << "," << end;
                QString expression = data.mid(offset, start - offset);
                QString sub = data.mid(start + 1, end - start - 1);
                expression = expression.trimmed();
                //kDebug() << "EXPR-MAIN " << expression;
                //kDebug() << "EXPR-SUB " << sub;

                QDomElement elem;
                if (cur.isNull()) {
                    elem = doc.createElement("ns:" + expression);
                    doc.appendChild(elem);
                } else {
                    elem = doc.createElement(expression);
                    cur.appendChild(elem);
                }

                buildtree(doc, elem, sub);

                offset = end + 1;
                start = -1;
                end = -1;
            }
        }
    }

    return doc;
}

#include "soap.moc"
