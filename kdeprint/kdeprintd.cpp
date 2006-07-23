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
#include <knotification.h>
#include <kmessagebox.h>
#include <kdebug.h>
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
#include <QtDBus/QtDBus>

#include <unistd.h>

extern "C"
{
	KDE_EXPORT KDEDModule *create_kdeprintd()
	{
		return new KDEPrintd();
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
    : QWidget(0, Qt::Window|Qt::MSWindowsFixedSizeDialogHint|Qt::WindowStaysOnTopHint), m_pid(pid)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setObjectName("StatusWindow");
	m_label = new QLabel(this);
	m_label->setAlignment(Qt::AlignCenter);
	m_button = new KPushButton(KStdGuiItem::close(), this);
	m_icon = new QLabel(this);
	m_icon->setPixmap(DesktopIcon("fileprint"));
	m_icon->setAlignment(Qt::AlignCenter);
#ifdef Q_WS_X11
	KWin::setIcons(winId(), *(m_icon->pixmap()), SmallIcon("fileprint"));
#endif
	QGridLayout	*l0 = new QGridLayout(this);
    l0->setMargin(10);
    l0->setSpacing(10);
	l0->setRowStretch(0, 1);
	l0->setColumnStretch(1, 1);
    l0->addWidget(m_label, 0, 1, 1, 2);
	l0->addWidget(m_button, 1, 2);
    l0->addWidget(m_icon, 0, 0, 2, 1);
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

KDEPrintd::KDEPrintd()
: KDEDModule()
{
}

KDEPrintd::~KDEPrintd()
{
  qDeleteAll(m_processpool);
  qDeleteAll(m_requestsPending);
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
	if ( re.indexIn( command ) != -1 )
	{
		KUrl url( re.cap( 1 ) );
		if ( !url.isLocalFile() )
		{
			QString tmpFilename = KStandardDirs::locateLocal( "tmp", "kdeprint_" + KRandom::randomString( 8 ) );
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
	m_processpool.removeAll( proc );
}

void KDEPrintd::slotPrintError( KPrintProcess *proc, const QString& msg )
{
	KNotification::event("printerror",i18n("<p><nobr>A print error occurred. Error message received from system:</nobr></p><br>%1", msg));
	m_processpool.removeAll( proc );
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
				i18n("Some of the files to printed are not readable by the KDE "
				     "print daemon. This may happen if you are trying to print "
				     "as a different user to the one currently logged in. To continue "
				     "printing, you need to provide root's password."),
				QString(),
				i18n("Enter the root password"),
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
			w->setWindowTitle(i18n("Printing Status - %1", QString("(pid=%1)").arg(pid)));
		else
			w->setWindowTitle(i18n("Printing Status - %1", appName));
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
	QString user;
	QString uri;
	QDBusMessage reply;
	int seqNbr;
};

QString KDEPrintd::requestPassword( const QString& user, const QString& host, int port, int seqNbr , const QDBusMessage& msg)
{
	Request *req = new Request;
	req->user = user;
	req->uri = "print://" + user + "@" + host + ":" + QString::number(port);
	req->seqNbr = seqNbr;
        msg.setDelayedReply(true);
	req->reply = msg;
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
	QByteArray params;
	QString authString( "::" );

	info.username = req->user;
	info.keepPassword = true;
	info.url = req->uri;
	info.comment = i18n( "Printing system" );

	QDataStream input( &params, QIODevice::WriteOnly );
	input << info;
	QDBusMessage reply =
		QDBusInterface( "org.kde.kded", "/modules/kpasswdserver", "org.kde.KPasswdServer" ).
		call("queryAuthInfo", params, i18n( "Authentication failed (user name=%1)",	 info.username ),
		     0, req->seqNbr);
	if ( reply.type() == QDBusMessage::ReplyMessage )
	{
		if ( reply.count() == 2 )
		{
			QDataStream output( reply.at(0).toByteArray() );
			int seqNbr = reply.at(1).toInt();
			KIO::AuthInfo result;
			output >> result;

			if ( result.isModified() )
				authString = result.username + ":" + result.password + ":" + QString::number( seqNbr );
		}
		else
			kWarning( 500 ) << "D-BUS returned invalid reply" << endl;
	}
	else
		kWarning( 500 ) << "Cannot communicate with kded_kpasswdserver" << endl;

	req->reply.sendReply(authString);

	m_requestsPending.removeAll( ( unsigned int )0 );
	if ( m_requestsPending.count() > 0 )
		QTimer::singleShot( 0, this, SLOT( processRequest() ) );
}

void KDEPrintd::initPassword( const QString& user, const QString& passwd, const QString& host, int port )
{
	QByteArray params;
	KIO::AuthInfo info;

	info.username = user;
	info.password = passwd;
	info.url = "print://" + user + "@" + host + ":" + QString::number(port);

	QDataStream input( &params, QIODevice::WriteOnly );
	input << info;

	QDBusMessage reply =
		QDBusInterface( "org.kde.kded", "/modules/kpasswdserver", "org.kde.KPasswdServer" ).
		call("addAuthInfo", params, qlonglong(0));
	if ( reply.type() != QDBusMessage::ReplyMessage )
		kWarning( 500 ) << "Unable to initialize password, cannot communicate with kded_kpasswdserver" << endl;
}

#include "kdeprintd.moc"
