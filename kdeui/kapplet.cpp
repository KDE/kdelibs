#include "qxembed.h"
#include "kapplet.h"
#include <unistd.h>
#include <dcopclient.h>
#include <kapp.h>
#include <qcstring.h>
#include <qdatastream.h>

class KAppletData
{
public:
    KAppletData()
    {
	orientation = Qt::Horizontal;
    };
    ~KAppletData()
    {
    }

    Qt::Orientation orientation;
};


KApplet::KApplet( QWidget* parent, const char* name  )
    : QWidget( parent, name ), DCOPObject()
{
    d = new KAppletData;
}

KApplet::~KApplet()
{
    delete d;
}

void KApplet::init( int& argc, char ** argv )
{
    int ideal_width = 42;
    int ideal_height = 42;

    DCOPClient* dcop = kapp->dcopClient();

    if (!dcop->isAttached() ) {
	if ( !dcop->attach() )
	    goto error;
    }

    // tell kicker that we are here and want be docked
    {
	QCString replyType;
	QByteArray data, replyData;
	QDataStream dataStream( data, IO_WriteOnly );
	dataStream << objId();

	// we use "call" to know whether it was really sucessful
	if ( !dcop->call("kicker", "appletArea", "dockMe(QCString)", data, replyType, replyData ) )
	    goto error;
    }

    return;

error:
    // do something, at least
    setupGeometry ( orientation(), ideal_width, ideal_height );
    show();
}


void KApplet::removeRequest()
{
    QByteArray data;
    QDataStream dataStream( data, IO_WriteOnly );
    dataStream << objId();
    kapp->dcopClient()->send("kicker", "appletArea", "removeMe(QCString)", data);
}

void KApplet::moveRequest()
{
    QByteArray data;
    QDataStream dataStream( data, IO_WriteOnly );
    dataStream << objId();
    kapp->dcopClient()->send("kicker", "appletArea", "moveMe(QCString)", data);
}


bool KApplet::process(const QCString &fun, const QByteArray &data,
	     QCString& replyType, QByteArray &replyData)
{
    if ( fun == "setupGeometry(int,int,int)" ) {
	QDataStream dataStream( data, IO_ReadOnly );
	int orient;
	int width, height;
	dataStream >> orient  >> width >> height;
	setupGeometry( Orientation(orient), width, height );
	return TRUE;
    } else if ( fun == "winId()" ) {
	QDataStream reply( replyData, IO_WriteOnly );
	reply << winId();
	replyType = "WId";
	return TRUE;
    } else if ( fun == "restartCommand()" ) {
	QDataStream reply( replyData, IO_WriteOnly );
	reply << QCString( kapp->argv()[0] );
	replyType = "QCString";
	return TRUE;
    }
    return FALSE;
}

void KApplet::setupGeometry( Orientation orientation , int width, int height  )
{
    QApplication::flushX();
    d->orientation = orientation;
    resize(width, height );
    updateGeometry();
}


QSize KApplet::sizeHint() const
{
    return size();
}

Qt::Orientation KApplet::orientation() const
{
    return d->orientation;
}

#include "kapplet.moc"
