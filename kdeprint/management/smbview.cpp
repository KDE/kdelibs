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

#include "smbview.h"

#include <kprocess.h>
#include <ktempfile.h>
#include <q3header.h>
#include <qapplication.h>
#include <QTextStream>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kcursor.h>

#include <qfile.h>
#include <qtextstream.h>
#include <cstdlib>


//*********************************************************************************************

SmbView::SmbView(QWidget *parent)
    : KListView(parent)
{
	addColumn(i18n("Printer"));
	addColumn(i18n("Comment"));
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	setAllColumnsShowFocus(true);
	setRootIsDecorated(true);

	m_state = Idle;
	m_current = 0;
	m_proc = new KProcess();
	m_proc->setUseShell(true);
	m_passwdFile = 0;
	connect(m_proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
	connect(m_proc,SIGNAL(receivedStdout(KProcess*,char*,int)),SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(this,SIGNAL(selectionChanged(Q3ListViewItem*)),SLOT(slotSelectionChanged(Q3ListViewItem*)));
}

SmbView::~SmbView()
{
	delete m_proc;
	delete m_passwdFile;
}

void SmbView::setLoginInfos(const QString& login, const QString& password)
{
	m_login = login;
	m_password = password;

	// We can't pass the password via the command line or the environment
	// because the command line is publically accessible on most OSes and
	// the environment is publically accessible on some OSes.
	// Therefor we write the password to a file and pass that file to
	// smbclient with the -A option
	delete m_passwdFile;
	m_passwdFile = new KTempFile;
	m_passwdFile->setAutoDelete(true);

	QTextStream *passwdFile = m_passwdFile->textStream();
	if (!passwdFile) return; // Error
	(*passwdFile) << "username = " << m_login << endl;
	(*passwdFile) << "password = " << m_password << endl;
	// (*passwdFile) << "domain = " << ???? << endl;

	m_passwdFile->close();
}

void SmbView::startProcess(int state)
{
	m_buffer.clear();
	m_state = state;
	QApplication::setOverrideCursor(KCursor::waitCursor());
	m_proc->start(KProcess::NotifyOnExit,KProcess::Stdout);
	emit running(true);
}

void SmbView::endProcess()
{
	switch (m_state)
	{
		case GroupListing:
			processGroups();
			break;
		case ServerListing:
			processServers();
			break;
		case ShareListing:
			processShares();
			break;
		default:
			break;
	}
	m_state = Idle;
	QApplication::restoreOverrideCursor();
	emit running(false);
	// clean up for future usage
	m_proc->clearArguments();
}

void SmbView::slotProcessExited(KProcess*)
{
	endProcess();
}

void SmbView::slotReceivedStdout(KProcess*, char *buf, int len)
{
	m_buffer.append(QString::fromLocal8Bit(buf,len));
}

void SmbView::init()
{
	// Open Samba configuration file and check if a WINS server is defined
	m_wins_server.clear();
	QLatin1String wins_keyword("wins server");	
	QFile smb_conf ("/etc/samba/smb.conf");
	if (smb_conf.exists () && smb_conf.open (QIODevice::ReadOnly))
	{
		QTextStream smb_stream (&smb_conf);
		while (!smb_stream.atEnd ())
		{
			QString smb_line = smb_stream.readLine ();
			if (smb_line.contains (wins_keyword, false) > 0)
			{
				QString key = smb_line.section('=', 0, 0);
				key = key.stripWhiteSpace();
				if (key.lower() != wins_keyword)
				{
					continue;
				}
				m_wins_server = smb_line.section ('=', 1, 1);
				// take only the first declared WINS server
				m_wins_server = m_wins_server.section(',', 0, 0);
				m_wins_server = m_wins_server.stripWhiteSpace ();
				m_wins_server = m_wins_server.section(' ', 0, 0);
				// strip any server tag (see man smb.conf(5))
				if (!m_wins_server.section(':', 1, 1).isNull())
				{
					m_wins_server = m_wins_server.section(':', 1, 1);
				}
				break;
			}
		}
		smb_conf.close ();
	}
	m_wins_server = m_wins_server.isEmpty ()? " " : " -U " + m_wins_server + " ";
	QString cmd ("nmblookup" + m_wins_server +
					"-M -- - | grep '<01>' | awk '{print $1}' | xargs nmblookup -A | grep '<1d>'");
	*m_proc << cmd;
	startProcess(GroupListing);
}

void SmbView::setOpen(Q3ListViewItem *item, bool on)
{
	if (on && item->childCount() == 0)
	{
		if (item->depth() == 0)
		{ // opening group
			m_current = item;
			*m_proc << "nmblookup"+m_wins_server+"-M ";
                        *m_proc << KProcess::quote(item->text(0));
                        *m_proc << " -S | grep '<20>' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*<20>.*//' | xargs -iserv_name smbclient -N -L 'serv_name' -W ";
                        *m_proc << KProcess::quote(item->text(0));
			*m_proc << " -A ";
                        *m_proc << KProcess::quote(m_passwdFile->name());
			startProcess(ServerListing);
		}
		else if (item->depth() == 1)
		{ // opening server
			m_current = item;
			*m_proc << "smbclient -N -L ";
                        *m_proc << KProcess::quote(item->text(0));
                        *m_proc << " -W ";
                        *m_proc << KProcess::quote(item->parent()->text(0));
			*m_proc << " -A ";
                        *m_proc << KProcess::quote(m_passwdFile->name());
			startProcess(ShareListing);
		}
	}
	Q3ListView::setOpen(item,on);
}

void SmbView::processGroups()
{
	QStringList	grps = QStringList::split('\n',m_buffer,false);
	clear();
	for (QStringList::ConstIterator it=grps.begin(); it!=grps.end(); ++it)
	{
		int	p = (*it).find("<1d>");
		if (p == -1)
			continue;
		Q3ListViewItem	*item = new Q3ListViewItem(this,(*it).left(p).trimmed());
		item->setExpandable(true);
		item->setPixmap(0,SmallIcon("network"));
	}
}

void SmbView::processServers()
{
	QStringList	lines = QStringList::split('\n',m_buffer,true);
	QString		line;
	int 		index(0);
	for (;index < lines.count();index++)
		if (lines[index].trimmed().startsWith("Server"))
			break;
	index += 2;
	while (index < lines.count())
	{
		line = lines[index++].trimmed();
		if (line.isEmpty())
			break;
		QStringList	words = QStringList::split(' ',line,false);
		Q3ListViewItem	*item = new Q3ListViewItem(m_current,words[0]);
		item->setExpandable(true);
		item->setPixmap(0,SmallIcon("kdeprint_computer"));
	}
}

void SmbView::processShares()
{
	QStringList	lines = QStringList::split('\n',m_buffer,true);
	QString		line;
	int 		index(0);
	for (;index < lines.count();index++)
		if (lines[index].trimmed().startsWith("Sharename"))
			break;
	index += 2;
	while (index < lines.count())
	{
		line = lines[index++].trimmed();
		if (line.isEmpty())
			break;
		else if ( line.startsWith( "Error returning" ) )
		{
			KMessageBox::error( this, line );
			break;
		}
		QString	typestr(line.mid(15, 10).trimmed());
		//QStringList	words = QStringList::split(' ',line,false);
		//if (words[1] == "Printer")
		if (typestr == "Printer")
		{
			QString	comm(line.mid(25).trimmed()), sharen(line.mid(0, 15).trimmed());
			//for (uint i=2; i<words.count(); i++)
			//	comm += (words[i]+" ");
			//QListViewItem	*item = new QListViewItem(m_current,words[0],comm);
			Q3ListViewItem	*item = new Q3ListViewItem(m_current,sharen,comm);
			item->setPixmap(0,SmallIcon("kdeprint_printer"));
		}
	}
}

void SmbView::slotSelectionChanged(Q3ListViewItem *item)
{
	if (item && item->depth() == 2)
		emit printerSelected(item->parent()->parent()->text(0),item->parent()->text(0),item->text(0));
}

void SmbView::abort()
{
	if (m_proc->isRunning())
		m_proc->kill();
}
#include "smbview.moc"
