/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
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
#include <qlabel.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kwin.h>
#include <qlayout.h>

#include <unistd.h>

extern "C"
{
	KDEDModule *create_kdeprintd(const QCString& name)
	{
		return new KDEPrintd(name);
	}
}

static void cleanFileList(const QStringList& files)
{
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		QFile::remove(*it);
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
: QWidget(NULL, "StatusWindow", WType_TopLevel|WStyle_DialogBorder|WStyle_StaysOnTop|WDestructiveClose), m_pid(pid)
{
	m_label = new QLabel(this);
	m_label->setAlignment(AlignCenter);
	m_button = new KPushButton(KStdGuiItem::close(), this);
	m_icon = new QLabel(this);
	m_icon->setPixmap(DesktopIcon("fileprint"));
	m_icon->setAlignment(AlignCenter);
	KWin::setIcons(winId(), *(m_icon->pixmap()), SmallIcon("fileprint"));
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

KDEPrintd::KDEPrintd(const QCString& obj)
: KDEDModule(obj)
{
	m_processpool.setAutoDelete(true);
	m_tempfiles.setAutoDelete(true);
	m_windows.setAutoDelete(false);
}

KDEPrintd::~KDEPrintd()
{
	cleanTempFiles();
}

int KDEPrintd::print(const QString& cmd, const QStringList& files, bool remflag)
{
	QString	command(cmd);
	if (!checkFiles(command, files))
		return (-1);

	KPrintProcess	*proc = new KPrintProcess;
	connect(proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
//	connect(proc,SIGNAL(passwordRequested(KProcess*,const QString&)),SLOT(slotPasswordRequested(KProcess*,const QString&)));
	*proc << command;
	if (remflag)
		m_tempfiles.insert(proc,new QStringList(files));
	if (proc->print())
	{
		m_processpool.append(proc);
		return (int)(proc->pid());
	}
	else
	{
		cleanTempFile(proc);
		delete proc;
		return (-1);
	}
}

void KDEPrintd::cleanTempFiles()
{
	QPtrDictIterator<QStringList>	it(m_tempfiles);
	for (;it.current();++it)
		cleanFileList(*(it.current()));
}

void KDEPrintd::cleanTempFile(KProcess *p)
{
	QStringList	*l = m_tempfiles.find(p);
	if (l)
		cleanFileList(*l);
}

void KDEPrintd::slotProcessExited(KProcess *proc)
{
	KPrintProcess	*pproc = (KPrintProcess*)proc;
	if (m_processpool.findRef(pproc) != -1)
	{
		m_processpool.take();
		QString		msg;
		if (!pproc->normalExit())
			msg = i18n("Abnormal process termination (<b>%1</b>).").arg(pproc->args().first());
		else if (pproc->exitStatus() != 0)
			msg = i18n("<b>%1</b>: execution failed with message:<p>%2</p>").arg(pproc->args().first()).arg(pproc->errorMessage());
		cleanTempFile(pproc);

		delete pproc;
		if (!msg.isEmpty())
			KNotifyClient::event("printerror",i18n("<p><nobr>A print error occured. Error message received from system:</nobr></p><br>%1").arg(msg));
	}
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
				QString::null,
				i18n("Provide root's password"),
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
	StatusWindow	*w = m_windows.find(pid);
	if (!w && !msg.isEmpty())
	{
		w = new StatusWindow(pid);
		if (appName.isEmpty())
			w->setCaption(i18n("Printing Status - %1").arg("(pid="+QString::number(pid)+")"));
		else
			w->setCaption(i18n("Printing Status - %1").arg(appName));
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

#include "kdeprintd.moc"
