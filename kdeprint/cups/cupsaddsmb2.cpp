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

#include <qtimer.h>
#include <qprogressdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <kstaticdeleter.h>
#include <kio/passdlg.h>
#include <kdebug.h>

#include <cups/cups.h>
#include <ctype.h>

CupsAddSmb*	CupsAddSmb::m_self = 0;
KStaticDeleter<CupsAddSmb>	m_cupsaddsmbsd;

CupsAddSmb::CupsAddSmb(QObject *parent, const char *name)
: QObject(parent, name)
{
	m_state = None;
	m_status = false;
	m_actionindex = 0;
	connect(&m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceived(KProcess*,char*,int)));
	connect(&m_proc, SIGNAL(receivedStderr(KProcess*,char*,int)), SLOT(slotReceived(KProcess*,char*,int)));
	connect(&m_proc, SIGNAL(processExited(KProcess*)), SLOT(slotProcessExited(KProcess*)));

	m_dlg = new QProgressDialog(0, "Dialog", true);
	m_dlg->setAutoClose(true);
	m_dlg->setMinimumDuration(0);
	connect(m_dlg, SIGNAL(cancelled()), SLOT(slotCancelled()));

	m_login = CupsInfos::self()->realLogin();
	m_password = CupsInfos::self()->password();
}

CupsAddSmb::~CupsAddSmb()
{
	delete m_dlg;
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
	if (m_actionindex < m_actions.count())
		QTimer::singleShot(1, this, SLOT(doNextAction()));
}

void CupsAddSmb::doNextAction()
{
	m_buffer.clear();
	m_state = None;
	if (m_proc.isRunning())
	{
		QCString	s = m_actions[m_actionindex++].latin1();
		m_dlg->setProgress(m_dlg->progress()+1);
		// kdDebug() << "NEXT ACTION = " << s << endl;
		if (s == "quit")
		{
			// do nothing
		}
		else if (s == "mkdir")
		{
			m_state = MkDir;
			m_dlg->setLabelText(i18n("Creating directory %1").arg(m_actions[m_actionindex]));
			s.append(" ").append(m_actions[m_actionindex].latin1());
			m_actionindex++;
		}
		else if (s == "put")
		{
			m_state = Copy;
			m_dlg->setLabelText(i18n("Uploading %1").arg(m_actions[m_actionindex+1]));
			s.append(" ").append(QFile::encodeName(m_actions[m_actionindex]).data()).append(" ").append(m_actions[m_actionindex+1].latin1());
			m_actionindex += 2;
		}
		else if (s == "adddriver")
		{
			m_state = AddDriver;
			m_dlg->setLabelText(i18n("Installing driver for %1").arg(m_actions[m_actionindex]));
			s.append(" \"").append(m_actions[m_actionindex].latin1()).append("\" \"").append(m_actions[m_actionindex+1].latin1()).append("\"");
			m_actionindex += 2;
		}
		else if (s == "addprinter")
		{
			m_state = AddPrinter;
			m_dlg->setLabelText(i18n("Installing printer %1").arg(m_actions[m_actionindex]));
			QCString	dest = m_actions[m_actionindex].local8Bit();
			s.append(" ").append(dest).append(" ").append(dest).append(" \"").append(dest).append("\" \"\"");
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
		if (qstrncmp(m_proc.args()->at(0), "smbclient", 9) == 0)
		{
			m_actions.clear();
			m_actions << "adddriver" << "Windows NT x86" << m_dest+":ADOBEPS5.DLL:"+m_dest+".PPD:ADOBEPSU.DLL:ADOBEPSU.HLP:NULL:RAW:NULL";
			m_actions << "addprinter" << m_dest;
			m_actions << "adddriver" << "Windows 4.0" << m_dest+":ADOBEPS4.DRV:"+m_dest+".PPD:NULL:ADOBEPS4.HLP:PSMON.DLL:RAW:ADFONTS.MFM,DEFPRTR2.PPD,ICONLIB.DLL";
			m_actions << "quit";

			doInstall();
			return;
		}
		else
		{
			m_dlg->reset();
			KMessageBox::information(0, i18n("Driver successfully exported."));
			return;
		}
	}

	m_dlg->reset();
	if (m_proc.normalExit())
	{
		if (QMessageBox::critical(0, i18n("Error"),
				i18n("Operation failed. Possible reasons are: permission denied "
				     "or invalid samba configuration (see <b>cupsaddsmb</b> manual "
					 "page for detailed information, needs at least cups-1.1.11). "
					 "Do you want to try with another login/password?"),
				QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Yes)
		{
			int	result = KIO::PasswordDialog::getNameAndPassword(m_login, m_password, 0);
			if (result == QDialog::Accepted)
			{
				// kdDebug() << "RESTARTING LAST PROCESS = " << m_proc.args()->at(0) << endl;
				if (qstrncmp(m_proc.args()->at(0), "smbclient", 9) == 0)
					doExport();
				else
					doInstall();
			}
		}
	}
	else
	{
		KMessageBox::error(0, i18n("Operation aborted."));
	}
}

void CupsAddSmb::slotCancelled()
{
	if (m_proc.isRunning())
		m_proc.kill();
}

bool CupsAddSmb::exportDest(const QString &dest, const QString& datadir)
{
	if (!m_self)
	{
		m_self = m_cupsaddsmbsd.setObject(m_self, new CupsAddSmb());
	}

	const char	*ppdfile;

	if ((ppdfile = cupsGetPPD(dest.local8Bit())) == NULL)
	{
		KMessageBox::error(0, "Missing files");
		return false;
	}

	m_self->m_actions.clear();
	m_self->m_actions << "mkdir" << "W32X86";
	m_self->m_actions << "put" << ppdfile << "W32X86/"+dest+".PPD";
	m_self->m_actions << "put" << datadir+"/drivers/ADOBEPS5.DLL" << "W32X86/ADOBEPS5.DLL";
	m_self->m_actions << "put" << datadir+"/drivers/ADOBEPSU.DLL" << "W32X86/ADOBEPSU.DLL";
	m_self->m_actions << "put" << datadir+"/drivers/ADOBEPSU.HLP" << "W32X86/ADOBEPSU.HLP";
	m_self->m_actions << "mkdir" << "WIN40";
	m_self->m_actions << "put" << ppdfile << "WIN40/"+dest+".PPD";
	m_self->m_actions << "put" << datadir+"/drivers/ADFONTS.MFM" << "WIN40/ADFONTS.MFM";
	m_self->m_actions << "put" << datadir+"/drivers/ADOBEPS4.DRV" << "WIN40/ADOBEPS4.DRV";
	m_self->m_actions << "put" << datadir+"/drivers/ADOBEPS4.HLP" << "WIN40/ADOBEPS4.HLP";
	m_self->m_actions << "put" << datadir+"/drivers/DEFPRTR2.PPD" << "WIN40/DEFPRTR2.PPD";
	m_self->m_actions << "put" << datadir+"/drivers/ICONLIB.DLL" << "WIN40/ICONLIB.DLL";
	m_self->m_actions << "put" << datadir+"/drivers/PSMON.DLL" << "WIN40/PSMON.DLL";
	m_self->m_actions << "quit";

	m_self->m_dest = dest;

	return m_self->doExport();
}

bool CupsAddSmb::doExport()
{
	m_status = false;
	m_state = None;

	m_self->m_dlg->setCaption(i18n("Export driver for %1").arg(m_dest));
	m_dlg->setTotalSteps(18);
	m_dlg->setLabelText(i18n("Preparing to upload driver files to %1").arg(cupsServer()));
	m_dlg->resize(350, 100);
	m_dlg->setProgress(0);

	m_proc.clearArguments();
	m_proc << "smbclient" << QString::fromLatin1("//")+cupsServer()+"/print$";
	return startProcess();
}

bool CupsAddSmb::doInstall()
{
	m_status = false;
	m_state = None;

	m_dlg->setLabelText(i18n("Preparing to install driver on %1").arg(cupsServer()));

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
