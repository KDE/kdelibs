/*

(C) Daniel M. Duley <mosfet@kde.org>
(C) Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "kpanelappmenu.h"
#include <qstringlist.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

static int panelmenu_get_seq_id()
{
    static int panelmenu_seq_no = -2;
    return panelmenu_seq_no--;
}


KPanelAppMenu::KPanelAppMenu(const QString &title, QObject *parent,
                       const char *name)
    : QObject(parent, name), DCOPObject()
{
    init(QString::null, title);
}

KPanelAppMenu::KPanelAppMenu(const QPixmap &icon, const QString &title,
                       QObject *parent, const char *name)
: QObject(parent, name), DCOPObject()
{

    init(icon, title);
}


KPanelAppMenu::KPanelAppMenu(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(name)
{
  realObjId = name;
}


void KPanelAppMenu::init(const QPixmap &icon, const QString &title)
{
    DCOPClient *client = kapp->dcopClient();
    if(!client->isAttached())
	client->attach();
    QByteArray sendData, replyData;
    QCString replyType;
    {
	QDataStream stream(sendData, IO_WriteOnly);
	stream << icon << title;
	if ( client->call("kicker", "kickerMenuManager", "createMenu(QPixmap,QString)", sendData, replyType, replyData ) ) {
	  if (replyType != "QCString")
	    kdDebug() << "error! replyType for createMenu should be QCstring in KPanelAppMenu::init" << endl;
	  else {
	    QDataStream reply( replyData, IO_ReadOnly );
	    reply >> realObjId;
	  }
	}
    }
    {
	QDataStream stream(sendData, IO_WriteOnly);
	stream << QCString("activated(int)") << client->appId() << objId();
	client->send("kicker", realObjId, "connectDCOPSignal(QCString,QCString,QCString)", sendData);
    }
}

KPanelAppMenu::~KPanelAppMenu()
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(sendData, IO_WriteOnly);
    stream << realObjId;
    client->send("kicker", "kickerMenuManager", "removeMenu", sendData );
}

int KPanelAppMenu::insertItem(const QPixmap &icon, const QString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(sendData, IO_WriteOnly);
    stream << icon << text << id;
    client->send("kicker", realObjId, "insertItem(QPixmap,QString,int)", sendData );
    return id;
}


KPanelAppMenu *KPanelAppMenu::insertMenu(const QPixmap &icon, const QString &text, int id )
{
    if ( id < 0 )
        id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData, replyData;
    QCString replyType;
    QDataStream stream(sendData, IO_WriteOnly);
    stream << icon << text << id;
    client->call("kicker", realObjId, "insertMenu(QPixmap,QString,int)", sendData, replyType, replyData );
    if ( replyType != "QCString")
      return 0;
    QDataStream ret(replyData, IO_ReadOnly);
    QCString subid;
    ret >> subid;

    QByteArray sendData2;
    QDataStream stream2(sendData2, IO_WriteOnly);
    stream2 << QCString("activated(int)") << client->appId() << subid;
    client->send("kicker", subid, "connectDCOPSignal(QCString,QCString,QCString)", sendData2);

    return new KPanelAppMenu(this, subid);
}


int KPanelAppMenu::insertItem(const QString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(sendData, IO_WriteOnly);
    stream << text << id;
    client->send("kicker", realObjId, "insertItem(QString,int)", sendData );
    return id;
}


void KPanelAppMenu::clear()
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    client->send("kicker", realObjId, "clear()", sendData);
}


bool KPanelAppMenu::process(const QCString &fun, const QByteArray &data,
			 QCString &replyType, QByteArray &)
{
    if ( fun == "activated(int)" ) {
	QDataStream dataStream( data, IO_ReadOnly );
	int id;
	dataStream >> id;
	emit activated( id );
	replyType = "void";
	return true;
    }
    return false;
}


#include "kpanelappmenu.moc"












