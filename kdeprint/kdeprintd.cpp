/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdeprintd.h"
#include "kprintprocess.h"

#include <qfile.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kio/passdlg.h>
#include <kio/authinfo.h>
#include <qlabel.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <krandom.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qregexp.h>

#include <unistd.h>

extern "C"
{
	KDE_EXPORT KDEDModule *create_kdeprintd(const QByteArray& name)
	{
		return new KDEPrintd(name);
	}
}

class StatusWindow : public QWidget
{
public:
	StatusWindow(int pid = -1);
	void setMessage(const QString&);
	int pid() const { return m_pid; }

private:
	QLabel		*m_label;
	QPushButton	*m_button;
	int		m_pid;
	QLabel		*m_icon;
};

StatusWindow::StatusWindow(int pid)
    : QWidget(0, Qt::WType_TopLevel|Qt::WStyle_DialogBorder|Qt::WStyle_StaysOnTop|Qt::WDestructiveClose), m_pid(pid)
{
    setObjectName( "StatusWindow" );
	m_label = new QLabel(this);
	m_label->setAlignment(Qt::AlignCenter);
	m_button = new KPushButton(KStdGuiItem::close(), this);
	m_icon = new QLabel(this);
	m_icon->setPixmap(DesktopIcon("fileprint"));
	m_icon->setAlignment(Qt::AlignCenter);
#ifdef Q_WS_X11
	KWin::setIcons(winId(), *(m_icon->pixmap()), SmallIcon("fileprint"));
#endif
	QGridLayout	*l0 = new QGridLayout(this, 2, 3, 10, 10);
	l0->setRowStretch(0, 1);
	l0->setColStretch(1, 1);
	l0->addMultiCellWidget(m_label, 0, 0, 1, 2);
	l0->addWidget(m_button, 1, 2);
	l0->addMultiCellWidget(m_icon, 0, 1, 0, 0);
	connect(m_button, SIGNAL(clicked()), SLOT(hide()));
	resize(200, 50);
}

void StatusWindow::setMessage(const QString& msg)
{
	//QSize	oldSz = size();
	m_label->setText(msg);
	//QSize	sz = m_label->sizeHint();
	//sz += QSize(layout()->margin()*2, layout()->margin()*2+layout()->spacing()+m_button->sizeHint().height());
	// dialog will never be smaller
	//sz = sz.expandedTo(oldSz);
	//resize(sz);
	//setFixedSize(sz);
	//layout()->activate();
}

//*****************************************************************************************************

KDEPrintd::KDEPrintd(const QByteArray& obj)
: KDEDModule(obj)
{
	m_processpool.setAutoDelete(true);
	m_requestsPending.setAutoDelete( true );
}

KDEPrintd::~KDEPrintd()
{
	m_windows.clear();
}

int KDEPrintd::print(const QString& cmd, const QStringList& files, bool remflag)
{
	KPrintProcess *proc = new KPrintProcess;
	QString	command(cmd);
	QRegExp re( "\\$out\\{([^}]*)\\}" );

	connect(proc,SIGNAL(printTerminated(KPrintProcess*)),SLOT(slotPrintTerminated(KPrintProcess*)));
	connect(proc,SIGNAL(printError(KPrintProcess*,const QString&)),SLOT(slotPrintError(KPrintProcess*,const QString&)));
	proc->setCommand( command );
	if ( re.search( command ) != -1 )
	{
		KUrl url( re.cap( 1 ) );
		if ( !url.isLocalFile() )
		{
			QString tmpFilename = locateLocal( "tmp", "kdeprint_" + KRandom::randomString( 8 ) );
			command.replace( re, KProcess::quote( tmpFilename ) );
			proc->setOutput( re.cap( 1 ) );
			proc->setTempOutput( tmpFilename );
		}
		else
			command.replace( re, KProcess::quote( re.cap( 1 ) ) );
	}

	if ( checkFiles( command, files ) )
	{
		*proc << command;
		if ( remflag )
			proc->setTempFiles( files );
		if ( proc->print() )
		{
			m_processpool.append( proc );
			return ( int )proc->pid();
		}
	}

	delete proc;
	return -1;
}

void KDEPrintd::slotPrintTerminated( KPrintProcess *proc )
{
	m_processpool.removeRef( proc );
}

void KDEPrintd::slotPrintError( KPrintProcess *proc, const QString& msg )
{
	KNotifyClient::event("printerror",i18n("<p><nobr>A print error occurred. Error message received from system:</nobr></p><br>%1").arg(msg));
	m_processpool.removeRef( proc );
}

QString KDEPrintd::openPassDlg(const QString& user)
{
	QString	user_(user), pass_, result;
	if (KIO::PasswordDialog::getNameAndPassword(user_, pass_, NULL) == KDialog::Accepted)
		result.append(user_).append(":").append(pass_);
	return result;
}

bool KDEPrintd::checkFiles(QString& cmd, const QStringList& files)
{
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (::access(QFile::encodeName(*it).data(), R_OK) != 0)
		{
			if (KMessageBox::warningContinueCancel(0,
				i18n("Some of the files to print are not readable by the KDE "
				     "print daemon. This may happen if you are trying to print "
				     "as a different user to the one currently logged in. To continue "
				     "printing, you need to provide root's password."),
				QString(),
				i18n("Provide root's Password"),
				"provideRootsPassword") == KMessageBox::Continue)
			{
				cmd = ("kdesu -c " + KProcess::quote(cmd));
				break;
			}
			else
				return false;
		}
	return true;
}

void KDEPrintd::statusMessage(const QString& msg, int pid, const QString& appName)
{
	StatusWindow	*w = m_windows.value(pid);
	if (!w && !msg.isEmpty())
	{
		w = new StatusWindow(pid);
		if (appName.isEmpty())
			w->setWindowTitle(i18n("Printing Status - %1").arg("(pid="+QString::number(pid)+")"));
		else
			w->setWindowTitle(i18n("Printing Status - %1").arg(appName));
		connect(w, SIGNAL(destroyed()), SLOT(slotClosed()));
		w->show();
		m_windows.insert(pid, w);
	}
	if (w)
	{
		if (!msg.isEmpty())
			w->setMessage(msg);
		else
			w->close();
	}
}

void KDEPrintd::slotClosed()
{
	const StatusWindow	*w = static_cast<const StatusWindow*>(sender());
	if (w)
	{
		m_windows.remove(w->pid());
	}
}

//******************************************************************************************

class KDEPrintd::Request
{
public:
	DCOPClientTransaction *transaction;
	QString user;
	QString uri;
	int seqNbr;
};

QString KDEPrintd::requestPassword( const QString& user, const QString& host, int port, int seqNbr )
{
	Request *req = new Request;
	req->user = user;
	req->uri = "print://" + user + "@" + host + ":" + QString::number(port);
	req->seqNbr = seqNbr;
	req->transaction = callingDcopClient()->beginTransaction();
	m_requestsPending.append( req );
	if ( m_requestsPending.count() == 1 )
		QTimer::singleShot( 0, this, SLOT( processRequest() ) );
	return "::";
}

void KDEPrintd::processRequest()
{
	if ( m_requestsPending.count() == 0 )
		return;

	Request *req = m_requestsPending.first();
	KIO::AuthInfo info;
	QByteArray params, reply;
	DCOPCString replyType;
	QString authString( "::" );

	info.username = req->user;
	info.keepPassword = true;
	info.url = req->uri;
	info.comment = i18n( "Printing system" );

	QDataStream input( &params, QIODevice::WriteOnly );
	input.setVersion ( QDataStream::Qt_3_1 );
	input << info << i18n( "Authentication failed (user name=%1)" ).arg( info.username ) << 0 << req->seqNbr;
	if ( callingDcopClient()->call( "kded", "kpasswdserver", "queryAuthInfo(KIO::AuthInfo,QString,long int,long int)",
				params, replyType, reply ) )
	{
		if ( replyType == "KIO::AuthInfo" )
		{
			QDataStream output( reply );
			output.setVersion ( QDataStream::Qt_3_1 );
			KIO::AuthInfo result;
			int seqNbr;
			output >> result >> seqNbr;

			if ( result.isModified() )
				authString = result.username + ":" + result.password + ":" + QString::number( seqNbr );
		}
		else
			kWarning( 500 ) << "DCOP returned type error, expected KIO::AuthInfo, received " << replyType << endl;
	}
	else
		kWarning( 500 ) << "Cannot communicate with kded_kpasswdserver" << endl;

	QByteArray outputData;
	QDataStream output( &outputData, QIODevice::WriteOnly );
	output.setVersion ( QDataStream::Qt_3_1 );
	output << authString;
	replyType = "QString";
	callingDcopClient()->endTransaction( req->transaction, replyType, outputData );

	m_requestsPending.remove( ( unsigned int )0 );
	if ( m_requestsPending.count() > 0 )
		QTimer::singleShot( 0, this, SLOT( processRequest() ) );
}

void KDEPrintd::initPassword( const QString& user, const QString& passwd, const QString& host, int port )
{
	QByteArray params, reply;
	DCOPCString replyType;
	KIO::AuthInfo info;

	info.username = user;
	info.password = passwd;
	info.url = "print://" + user + "@" + host + ":" + QString::number(port);

	QDataStream input( &params, QIODevice::WriteOnly );
	input.setVersion ( QDataStream::Qt_3_1 );
	input << info << ( long int )0;

	if ( !callingDcopClient()->call( "kded", "kpasswdserver", "addAuthInfo(KIO::AuthInfo,long int)",
			params, replyType, reply ) )
		kWarning( 500 ) << "Unable to initialize password, cannot communicate with kded_kpasswdserver" << endl;
}

#include "kdeprintd.moc"
