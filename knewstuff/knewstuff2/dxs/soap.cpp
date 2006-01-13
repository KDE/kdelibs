#include "soap.h"

#include <qdom.h>
#include <qsocket.h>
#include <qsocketnotifier.h>
#include <qregexp.h>

#include <kio/job.h>
#include <kurl.h>
#include <kdebug.h>

#include <string.h> // memcpy()

using namespace KNS;

Soap::Soap()
: QObject()
{
	m_model = canonicaltree;
	m_socket = NULL;
	m_inprogress = false;
}

Soap::~Soap()
{
}

void Soap::call(QDomElement element, QString endpoint)
{
	if(m_inprogress) return;

	if(m_model == canonicaltree)
	{
		call_tree(element, endpoint);
	}
	else
	{
		call_soap(element, endpoint);
	}

	m_inprogress = true;
}

void Soap::call_tree(QDomElement element, QString endpoint)
{
	Q_UNUSED(endpoint);

	QString s = QString();

	s += localname(element);
	s += "(";
	QDomNodeList l = element.childNodes();
	for(unsigned int i = 0; i < l.count(); i++)
	{
		QDomNode tmp = l.item(i);
		s += localname(tmp);
		s += "(";
		s += xpath(tmp, "/");
		s += ")";
		s += "\n";
	}
	s += ")\n";

	kdDebug() << "<CanonicalTree>" << s << endl;

	QByteArray data = s.utf8();
	data.truncate(data.size() - 1);

	kdDebug() << "Call(socket)!" << endl;

	m_socket = new QSocket();
	m_socket->connectToHost("localhost", 10000);

	m_socket->writeBlock(data, data.size());

	connect(m_socket, SIGNAL(readyRead()), SLOT(slotSocket()));
	connect(m_socket, SIGNAL(error(int)), SLOT(slotSocketError(int)));

	m_buffer = QByteArray();
}

void Soap::call_soap(QDomElement element, QString endpoint)
{
	KURL url(endpoint);

	QDomDocument doc;
	QDomElement env = doc.createElement("SOAP-ENV:Envelope");
	env.setAttribute("xmlns:SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/");
	doc.appendChild(env);
	QDomElement body = doc.createElement("SOAP-ENV:Body");
	env.appendChild(body);
	element.setAttribute("xmlns:ns", "urn:GHNS");
	body.appendChild(element);

	QString s = doc.toString();

	QByteArray data = s.utf8();
	data.truncate(data.size() - 1);

	kdDebug() << "HTTP-POST" << url.prettyURL() << endl;
	kdDebug() << "HTTP-POST" << data << endl;

	KIO::Job *job;
	job = KIO::http_post(url, data, false);
	job->addMetaData("content-type", "Content-Type: text/xml");

	kdDebug() << "Call!" << endl;

	connect(job,
		SIGNAL(data(KIO::Job*, const QByteArray&)),
		SLOT(slotData(KIO::Job*, const QByteArray&)));
	connect(job,
		SIGNAL(result(KIO::Job*)),
		SLOT(slotResult(KIO::Job*)));

	m_buffer = QByteArray();
}

void Soap::slotData(KIO::Job *job, const QByteArray& data)
{
	Q_UNUSED(job);

	kdDebug() << "Length(before): " << m_buffer.size() << endl;
	kdDebug() << "Append-Length: " << data.size() << endl;

	int bufferlen = m_buffer.size();
	m_buffer.resize(bufferlen + data.size());

	//m_buffer.append(data);
	// FIXME: No memcpy() in Qt??? Really, no qMemCopy()? :-)
	memcpy(m_buffer.data() + bufferlen, data.data(), data.size());

	kdDebug() << "Length(after): " << m_buffer.size() << endl;
}

void Soap::slotResult(KIO::Job *job)
{
	kdDebug() << "Result!" << endl;

	if((job) && (job->error()))
	{
		//job->showErrorDialog(this);
		return;
	}

	kdDebug() << "CSTRING: '" << m_buffer << "'" << endl;

	int bufferlen = m_buffer.size();
	m_buffer.resize(bufferlen + 1);
	m_buffer.data()[bufferlen] = 0;
	m_data = QString::fromUtf8(m_buffer);

	kdDebug() << "STRING: '" << m_data << "'" << endl;

	if(m_model == soap)
	{
		QDomDocument doc;
		doc.setContent(m_data);

		QDomElement envelope = doc.documentElement();
		QDomNode bodynode = envelope.firstChild();
		QDomNode contentnode = bodynode.firstChild();

		kdDebug() << "(signal) Result!" << endl;

		emit signalResult(contentnode);
		m_inprogress = false;
	}
	else
	{
		QDomDocument doc;

		// FIXME: dummy data
		//m_data = QString("GHNSRemovalResponse(ok(true)\nauthorative(true))");
		kdDebug() << m_data << endl;

		doc = buildtree(doc, doc.documentElement(), m_data);

		QDomElement root = doc.documentElement();

		kdDebug() << "(signal) Result!" << endl;
		kdDebug() << doc.toString() << endl;

		emit signalResult(root);
		m_inprogress = false;
	}
}

QString Soap::localname(QDomNode node)
{
	QDomElement el = node.toElement();
	QString s = el.tagName().section(":", -1);
	return s;
}

QString Soap::xpath(QDomNode node, QString expr)
{
	if(m_model == canonicaltree)
	{
		//QString provider = m_soap->xpath(node, "/SOAP-ENC:Array/provider");
		expr = expr.section("/", 2);
		// FIXME: Array handling for Canonical Tree Structures?
		kdDebug() << "EXPR " << expr << endl;
	}

	QDomNode n = node;
	QStringList explist = QStringList::split("/", expr);
	for(QStringList::Iterator it = explist.begin(); it != explist.end(); it++)
	{
		QDomElement el = n.toElement();
		QDomNodeList l = el.elementsByTagName((*it));
		n = l.item(0);
	}
	QString s = n.toElement().text();
	return s;
}

void Soap::setModel(Model m)
{
	m_model = m;
}

void Soap::slotSocketError(int error)
{
	Q_UNUSED(error);

	kdDebug() << "socket: error" << endl;

//	delete m_socket;
	m_socket = NULL;

	m_inprogress = false;
}

void Soap::slotSocket()
{
	kdDebug() << "socket: data" << endl;

	QByteArray a(m_socket->bytesAvailable());
	m_socket->readBlock(a.data(), m_socket->bytesAvailable());

	kdDebug() << "DATA" << a << endl;

	slotData(NULL, a);

	if(m_socket->atEnd())
	{
		m_socket->close();
//		delete m_socket;
		m_socket = NULL;

		slotResult(NULL);
	}
}

QDomDocument Soap::buildtree(QDomDocument doc, QDomElement cur, QString data)
{
	kdDebug() << "MATCH " << data << endl;

	//QRegExp e("(\\S+)\\(((?:.*\\n?)+)\\)");
	//QRegExp e("^(\\S+)\\((.*)\\)$"); // FIXME: Qt regexp bug?
	//QRegExp e("^([^(]+)\\((.*)\\)$");
	//QRegExp e("^([^(]+)(?:\\((.*)\\))?\n*$");
	//QRegExp e("^([^\(]+(?!\()[^\\\(])(?:\\((.*)[^\\]\\))?\n*$");
	//QRegExp e("^([^\\(]+[^\\\\\\(])(?:\\((.*[^\\\\])\\))?\n*$");
	//QRegExp e("^((?:\\\\\\(|[^\\(])+)(?:\\(((?:\\\\\\)|\\\\\\(|.)*)\\))?\n*$");
	QRegExp e("^((?:\\\\\\(|[^\\(])+)(?:\\(((?:\\\\\\)|.)*)\\))?\n*$");
	e.match(data);
	//kdDebug() << "Captures: " << e.numCaptures() << endl;
	kdDebug() << "Captures: " << e.numCaptures() << endl;
	//for(int i = 0; i < e.numCaptures(); i++)
	//{
	//	kdDebug() << "Cap(" << i + 1 << "): " << e.cap(i + 1) << endl;
	//}

	// FIXME: another Qt regexp bug?
	//if(e.numCaptures() == 2)
//kdDebug() << "cap1:" << e.cap(1) << endl;
	if(data.contains(QRegExp("[^\\\\]\\(")))
	{
//kdDebug() << "contains-parentheses" << endl;
		QDomElement elem;
		if(cur.isNull())
		{
			elem = doc.createElement("ns:" + e.cap(1));
			doc.appendChild(elem);
		}
		else
		{
			elem = doc.createElement(e.cap(1));
			cur.appendChild(elem);
		}

		QStringList l = QStringList::split("\n", e.cap(2));
		for(QStringList::iterator it = l.begin(); it != l.end(); it++)
		{
			kdDebug() << "<rec>" << (*it) << endl;
			buildtree(doc, elem, (*it));
		}
	}
	else //if(e.numCaptures() == 1)
	{
		QString text = e.cap(1);
		//text = text.replace(QRegExp("\\\\(?:[^\\\\]|$)"), "");
		text = text.replace(QRegExp("\\\\\\("), "(");
		text = text.replace(QRegExp("\\\\\\)"), ")");
		QDomText t = doc.createTextNode(text);
		cur.appendChild(t);
		//cur.setNodeValue(e.cap(1));
	}

	return doc;
}

#include "soap.moc"
