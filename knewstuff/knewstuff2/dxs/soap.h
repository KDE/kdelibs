/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
class QDomNodeList;
class QDomElement;
class QTcpSocket;
class QCString;

namespace KNS
{

class Soap : public QObject
{
Q_OBJECT
public:
	Soap();
	~Soap();

	enum Model
	{
		soap,
		canonicaltree
	};

	void setModel(Model m);

	void call(QDomElement element, QString endpoint);
	QString localname(QDomNode node);
	QString xpath(QDomNode node, QString expr);
	QList<QDomNode> directChildNodes(QDomNode node, QString name);

signals:
	void signalResult(QDomNode node);
	void signalError();

private slots:
	void slotData(KIO::Job *job, const QByteArray& data);
	void slotResult(KJob *job);
	void slotSocket();
	void slotSocketError(QAbstractSocket::SocketError error);

private:
	void call_soap(QDomElement element, QString endpoint);
	void call_tree(QDomElement element, QString endpoint);
	QDomDocument buildtree(QDomDocument doc, QDomElement cur, QString data);

	QString m_data;
	Model m_model;
	QTcpSocket *m_socket;
	QByteArray m_buffer;
	bool m_inprogress;
};

}

#endif
