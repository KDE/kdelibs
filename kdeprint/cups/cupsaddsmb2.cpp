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

#include "cupsaddsmb2.h"
#include "cupsinfos.h"
#include "sidepixmap.h"

#include <qtimer.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <kio/passdlg.h>
#include <kdebug.h>
#include <kseparator.h>
#include <kactivelabel.h>

#include <cups/cups.h>
#include <ctype.h>

CupsAddSmb::CupsAddSmb(QWidget *parent, const char *name)
: KDialog(parent, name)
{
	m_state = None;
	m_status = false;
	m_actionindex = 0;
	connect(&m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceived(KProcess*,char*,int)));
	connect(&m_proc, SIGNAL(receivedStderr(KProcess*,char*,int)), SLOT(slotReceived(KProcess*,char*,int)));
	connect(&m_proc, SIGNAL(processExited(KProcess*)), SLOT(slotProcessExited(KProcess*)));

	m_side = new SidePixmap(this);
	m_doit = new QPushButton(i18n("&Export"), this);
	m_cancel = new QPushButton(i18n("&Cancel"), this);
	m_passbtn = new QPushButton(i18n("Change &Login/Password"), this);
	connect(m_cancel, SIGNAL(clicked()), SLOT(reject()));
	connect(m_doit, SIGNAL(clicked()), SLOT(slotActionClicked()));
	connect(m_passbtn, SIGNAL(clicked()), SLOT(slotChangePassword()));
	m_bar = new QProgressBar(this);
	m_text = new KActiveLabel(this);
	QLabel	*m_title = new QLabel(i18n("Export printer driver to Windows clients"), this);
	setCaption(m_title->text());
	QFont	f(m_title->font());
	f.setBold(true);
	m_title->setFont(f);
	KSeparator	*m_sep = new KSeparator(Qt::Horizontal, this);

	QHBoxLayout	*l0 = new QHBoxLayout(this, 10, 10);
	QVBoxLayout	*l1 = new QVBoxLayout(0, 0, 10);
	l0->addWidget(m_side);
	l0->addLayout(l1);
	l1->addWidget(m_title);
	l1->addWidget(m_sep);
	l1->addWidget(m_text, 1);
	l1->addWidget(m_bar);
	l1->addSpacing(50);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addStretch(1);
	l2->addWidget(m_passbtn);
	l2->addWidget(m_doit);
	l2->addWidget(m_cancel);

	m_login = CupsInfos::self()->realLogin();
	m_password = CupsInfos::self()->password();

	setMinimumHeight(350);
}

CupsAddSmb::~CupsAddSmb()
{
}

void CupsAddSmb::slotActionClicked()
{
	if (m_state == None)
		doExport();
	else if (m_proc.isRunning())
		m_proc.kill();
}

void CupsAddSmb::slotReceived(KProcess*, char *buf, int buflen)
{
	QString	line;
	int		index(0);
	bool	partial(false);
	static bool incomplete(false);

	// kdDebug() << "slotReceived()" << endl;
	while (1)
	{
		// read a line
		line = QString::fromLatin1("");
		partial = true;
		while (index < buflen)
		{
			QChar	c(buf[index++]);
			if (c == '\n')
			{
				partial = false;
				break;
			}
			else if (c.isPrint())
				line += c;
		}

		if (line.isEmpty())
		{
			// kdDebug() << "NOTHING TO READ" << endl;
			return;
		}

		// kdDebug() << "ANSWER = " << line << " (END = " << line.length() << ")" << endl;
		if (!partial)
		{
			if (incomplete && m_buffer.count() > 0)
				m_buffer[m_buffer.size()-1].append(line);
			else
				m_buffer << line;
			incomplete = false;
			// kdDebug() << "COMPLETE LINE READ (" << m_buffer.count() << ")" << endl;
		}
		else
		{
			if (line.startsWith("smb:") || line.startsWith("rpcclient $"))
			{
				// kdDebug() << "END OF ACTION" << endl;
				checkActionStatus();
				if (m_status)
					nextAction();
				else
				{
					// quit program
					// kdDebug() << "EXITING PROGRAM..." << endl;
					m_proc.writeStdin("quit\n", 5);
					// kdDebug() << "SENT" << endl;
				}
				return;
			}
			else
			{
				if (incomplete && m_buffer.count() > 0)
					m_buffer[m_buffer.size()-1].append(line);
				else
					m_buffer << line;
				incomplete = true;
				// kdDebug() << "INCOMPLETE LINE READ (" << m_buffer.count() << ")" << endl;
			}
		}
	}
}

void CupsAddSmb::checkActionStatus()
{
	m_status = false;
	// when checking the status, we need to take into account the
	// echo of the command in the output buffer.
	switch (m_state)
	{
		case None:
		case Start:
			m_status = true;
			break;
		case Copy:
			m_status = (m_buffer.count() == 1);
			break;
		case MkDir:
			m_status = (m_buffer.count() == 1 || m_buffer[1].find("ERRfilexists") != -1);
			break;
		case AddDriver:
		case AddPrinter:
			m_status = (m_buffer.count() == 1 || !m_buffer[1].startsWith("result"));
			break;
	}
	// kdDebug() << "ACTION STATUS = " << m_status << endl;
}

void CupsAddSmb::nextAction()
{
	if (m_actionindex < (int)(m_actions.count()))
		QTimer::singleShot(1, this, SLOT(doNextAction()));
}

void CupsAddSmb::doNextAction()
{
	m_buffer.clear();
	m_state = None;
	if (m_proc.isRunning())
	{
		QCString	s = m_actions[m_actionindex++].latin1();
		m_bar->setProgress(m_bar->progress()+1);
		// kdDebug() << "NEXT ACTION = " << s << endl;
		if (s == "quit")
		{
			// do nothing
		}
		else if (s == "mkdir")
		{
			m_state = MkDir;
			m_text->setText(i18n("Creating directory %1").arg(m_actions[m_actionindex]));
			s.append(" ").append(m_actions[m_actionindex].latin1());
			m_actionindex++;
		}
		else if (s == "put")
		{
			m_state = Copy;
			m_text->setText(i18n("Uploading %1").arg(m_actions[m_actionindex+1]));
			s.append(" ").append(QFile::encodeName(m_actions[m_actionindex]).data()).append(" ").append(m_actions[m_actionindex+1].latin1());
			m_actionindex += 2;
		}
		else if (s == "adddriver")
		{
			m_state = AddDriver;
			m_text->setText(i18n("Installing driver for %1").arg(m_actions[m_actionindex]));
			s.append(" \"").append(m_actions[m_actionindex].latin1()).append("\" \"").append(m_actions[m_actionindex+1].latin1()).append("\"");
			m_actionindex += 2;
		}
		else if (s == "addprinter" || s == "setdriver")
		{
			m_state = AddPrinter;
			m_text->setText(i18n("Installing printer %1").arg(m_actions[m_actionindex]));
			QCString	dest = m_actions[m_actionindex].local8Bit();
			if (s == "addprinter")
				s.append(" ").append(dest).append(" ").append(dest).append(" \"").append(dest).append("\" \"\"");
			else
				s.append(" ").append(dest).append(" ").append(dest);
			m_actionindex++;
		}
		else
		{
			// kdDebug() << "ACTION = unknown action" << endl;
			m_proc.kill();
			return;
		}
		// send action
		// kdDebug() << "ACTION = " << s << endl;
		s.append("\n");
		m_proc.writeStdin(s.data(), s.length());
	}
}

void CupsAddSmb::slotProcessExited(KProcess*)
{
	// kdDebug() << "PROCESS EXITED (" << m_state << ")" << endl;
	if (m_proc.normalExit() && m_state != Start && m_status)
	{
		// last process went OK. If it was smbclient, then switch to rpcclient
		if (qstrncmp(m_proc.args().first(), "smbclient", 9) == 0)
		{
			doInstall();
			return;
		}
		else
		{
			m_doit->setEnabled(false);
			m_passbtn->setEnabled(false);
			m_cancel->setEnabled(true);
			m_cancel->setText(i18n("Close"));
			m_cancel->setDefault(true);
			m_cancel->setFocus();
			m_text->setText(i18n("Driver successfully exported."));
			return;
		}
	}

	if (m_proc.normalExit())
	{
		showError(
				i18n("Operation failed. Possible reasons are: permission denied "
				     "or invalid Samba configuration (see <a href=\"man:/cupsaddsmb\">"
					 "cupsaddsmb</a> manual page for detailed information, you need "
					 "<a href=\"http://www.cups.org\">CUPS</a> version 1.1.11 or higher). "
					 "You may want to try again with another login/password."));

	}
	else
	{
		showError(i18n("Operation aborted (process killed)."));
	}
}

void CupsAddSmb::slotChangePassword()
{
	KIO::PasswordDialog::getNameAndPassword(m_login, m_password, 0);
}

void CupsAddSmb::showError(const QString& msg)
{
	m_text->setText(i18n("<h1>Operation failed !</h1><p>%1</p>").arg(msg));
	m_cancel->setEnabled(true);
	m_passbtn->setEnabled(true);
	m_doit->setText(i18n("Export"));
	m_state = None;
}

bool CupsAddSmb::exportDest(const QString &dest, const QString& datadir)
{
	CupsAddSmb	dlg;
	dlg.m_dest = dest;
	dlg.m_datadir = datadir;
	dlg.m_text->setText(
			i18n("You are about to export the <b>%1</b> driver to a Windows client "
				 "through Samba. This operation requires the <a href=\"http://www.adobe.com\">"
				 "Adobe PostScript Driver</a>, <a href=\"http://www.samba.org\">Samba</a> "
				 "version 2.2 and a running SMB service on server <b>%1</b>. Click <b>Export</b> "
				 "to start the operation.").arg(dest).arg(cupsServer()));
	dlg.exec();
	return dlg.m_status;
}

bool CupsAddSmb::doExport()
{
	m_status = false;
	m_state = None;

	if (!QFile::exists(m_datadir+"/drivers/ADOBEPS5.DLL") ||
	    !QFile::exists(m_datadir+"/drivers/ADOBEPS4.DRV"))
	{
		showError(
			i18n("Some driver files are missing. You can get them on "
			     "<a href=\"http://www.adobe.com\">Adobe</a> web site. "
			     "See <a href=\"man:/cupsaddsmb\">cupsaddsmb</a> manual "
				 "page for more details (you need <a href=\"http://www.cups.org\">CUPS</a> "
				 "version 1.1.11 or higher)."));
		return false;
	}

	m_bar->setTotalSteps(18);
	m_bar->setProgress(0);
	m_text->setText(i18n("<p>Preparing to upload driver to host <b>%1</b>").arg(cupsServer()));
	m_cancel->setEnabled(false);
	m_passbtn->setEnabled(false);
	m_doit->setText(i18n("Abort"));

	const char	*ppdfile;

	if ((ppdfile = cupsGetPPD(m_dest.local8Bit())) == NULL)
	{
		showError(i18n("The driver for printer <b>%1</b> could not be found.").arg(m_dest));
		return false;
	}

	m_actions.clear();
	m_actions << "mkdir" << "W32X86";
	m_actions << "put" << ppdfile << "W32X86/"+m_dest+".PPD";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS5.DLL" << "W32X86/ADOBEPS5.DLL";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPSU.DLL" << "W32X86/ADOBEPSU.DLL";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPSU.HLP" << "W32X86/ADOBEPSU.HLP";
	m_actions << "mkdir" << "WIN40";
	m_actions << "put" << ppdfile << "WIN40/"+m_dest+".PPD";
	m_actions << "put" << m_datadir+"/drivers/ADFONTS.MFM" << "WIN40/ADFONTS.MFM";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS4.DRV" << "WIN40/ADOBEPS4.DRV";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS4.HLP" << "WIN40/ADOBEPS4.HLP";
	m_actions << "put" << m_datadir+"/drivers/DEFPRTR2.PPD" << "WIN40/DEFPRTR2.PPD";
	m_actions << "put" << m_datadir+"/drivers/ICONLIB.DLL" << "WIN40/ICONLIB.DLL";
	m_actions << "put" << m_datadir+"/drivers/PSMON.DLL" << "WIN40/PSMON.DLL";
	m_actions << "quit";

	m_proc.clearArguments();
	m_proc << "smbclient" << QString::fromLatin1("//")+cupsServer()+"/print$";
	return startProcess();
}

bool CupsAddSmb::doInstall()
{
	m_status = false;
	m_state = None;

	m_actions.clear();
	m_actions << "adddriver" << "Windows NT x86" << m_dest+":ADOBEPS5.DLL:"+m_dest+".PPD:ADOBEPSU.DLL:ADOBEPSU.HLP:NULL:RAW:NULL";
	// seems to be wrong with newer versions of Samba
	//m_actions << "addprinter" << m_dest;
	m_actions << "adddriver" << "Windows 4.0" << m_dest+":ADOBEPS4.DRV:"+m_dest+".PPD:NULL:ADOBEPS4.HLP:PSMON.DLL:RAW:ADFONTS.MFM,DEFPRTR2.PPD,ICONLIB.DLL";
	// seems to be ok with newer versions of Samba
	m_actions << "setdriver" << m_dest;
	m_actions << "quit";

	m_text->setText(i18n("Preparing to install driver on host <b>%1</b>").arg(cupsServer()));

	m_proc.clearArguments();
	m_proc << "rpcclient" << QString::fromLatin1(cupsServer());
	return startProcess();
}

bool CupsAddSmb::startProcess()
{
	m_proc << "-d" << "0" << "-N" << "-U";
	if (m_password.isEmpty())
		m_proc << m_login;
	else
		m_proc << m_login+"%"+m_password;
	m_state = Start;
	m_actionindex = 0;
	m_buffer.clear();
	// kdDebug() << "PROCESS STARTED = " << m_proc.args()->at(0) << endl;
	return m_proc.start(KProcess::NotifyOnExit, KProcess::All);
}

#include "cupsaddsmb2.moc"
