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

#include "escpwidget.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <klibloader.h>

class EscpFactory : public KLibFactory
{
public:
	EscpFactory(QObject *parent = 0, const char *name = 0) : KLibFactory(parent, name) {}
protected:
	QObject* createObject(QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList& args = QStringList())
	{
		KDialogBase	*dlg = new KDialogBase(static_cast<QWidget*>(parent), name, true, i18n("EPSON InkJet Printer Utilities"), KDialogBase::Close);
		EscpWidget	*w = new EscpWidget(dlg);
		if (args.count() > 0)
			w->setDevice(args[0]);
		dlg->setMainWidget(w);
		return dlg;
	}
};

extern "C"
{
	void* init_libescputil()
	{
		return new EscpFactory;
	}
};

EscpWidget::EscpWidget(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	connect(&m_proc, SIGNAL(processExited(KProcess*)), SLOT(slotProcessExited(KProcess*)));
	connect(&m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(&m_proc, SIGNAL(receivedStderr(KProcess*,char*,int)), SLOT(slotReceivedStderr(KProcess*,char*,int)));

	QPushButton	*cleanbtn = new QPushButton(this, "-c");
	cleanbtn->setPixmap(DesktopIcon("exec"));
	QPushButton	*nozzlebtn = new QPushButton(this, "-n");
	nozzlebtn->setPixmap(DesktopIcon("exec"));
	QPushButton	*alignbtn = new QPushButton(this, "-a");
	alignbtn->setPixmap(DesktopIcon("exec"));
	QPushButton	*inkbtn = new QPushButton(this, "-i");
	inkbtn->setPixmap(DesktopIcon("kdeprint_inklevel"));
	QPushButton	*identbtn = new QPushButton(this, "-d");
	identbtn->setPixmap(DesktopIcon("exec"));

	connect(cleanbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(nozzlebtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(alignbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(inkbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(identbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));

	QLabel	*cleanlab = new QLabel(i18n("Clean print head"), this);
	QLabel	*nozzlelab = new QLabel(i18n("Print a nozzle test pattern"), this);
	QLabel	*alignlab = new QLabel(i18n("Align print head"), this);
	QLabel	*inklab = new QLabel(i18n("Ink level"), this);
	QLabel	*identlab = new QLabel(i18n("Printer identification"), this);

	QGridLayout	*l0 = new QGridLayout(this, 5, 2, 10, 10);
	l0->addWidget(cleanbtn, 0, 0);
	l0->addWidget(nozzlebtn, 1, 0);
	l0->addWidget(alignbtn, 2, 0);
	l0->addWidget(inkbtn, 3, 0);
	l0->addWidget(identbtn, 4, 0);
	l0->addWidget(cleanlab, 0, 1);
	l0->addWidget(nozzlelab, 1, 1);
	l0->addWidget(alignlab, 2, 1);
	l0->addWidget(inklab, 3, 1);
	l0->addWidget(identlab, 4, 1);
	l0->setColStretch(1, 1);
}

void EscpWidget::startCommand(const QString& arg)
{
	bool	useUSB(false);

	if (m_device.isEmpty())
	{
		KMessageBox::error(this, i18n("Internal error: no device set."));
		return;
	}
	else
	{
		QString	protocol = m_device.protocol();
		if (protocol == "usb")
			useUSB = true;
		else if (protocol != "file" && protocol != "parallel" && protocol != "serial" && !protocol.isEmpty())
		{
			KMessageBox::error(this,
				i18n("Unsupported connection type: %1").arg(protocol));
			return;
		}
	}

	if (m_proc.isRunning())
	{
		KMessageBox::error(this, i18n("<p>An <b>escputil</b> process is still running. "
		                              "You must wait until its completion before continuing.</p>"));
		return;
	}

	QString	exestr = KStandardDirs::findExe("escputil");
	if (exestr.isEmpty())
	{
		KMessageBox::error(this, i18n("<p>The executable <b>escputil</b> cannot be found in your "
		                              "PATH environment variable. Make sure gimp-print is "
		                              "installed and that escputil is in your PATH.</p>"));
		return;
	}

	m_proc.clearArguments();
	m_proc << exestr << "-r" << m_device.path();
	if (useUSB)
		m_proc << "-u";

	m_proc << arg << "-q";
	m_errorbuffer = m_outbuffer = QString::null;
	if (m_proc.start(KProcess::NotifyOnExit, KProcess::AllOutput))
		setEnabled(false);
	else
	{
		KMessageBox::error(this,
			i18n("Internal error: unable to start escputil process."));
		return;
	}
}

void EscpWidget::slotProcessExited(KProcess*)
{
	setEnabled(true);
	if (!m_proc.normalExit() || m_proc.exitStatus() != 0)
	{
		QString	msg1 = "<nobr>"+i18n("Operation terminated with errors.")+"</nobr>";
		QString	msg2;
		if (!m_outbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Output")+"</u></b></p><p>"+m_outbuffer+"</p>";
		if (!m_errorbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Error")+"</u></b></p><p>"+m_errorbuffer+"</p>";
		if (!msg2.isEmpty())
			KMessageBox::detailedError(this, msg1, msg2);
		else
			KMessageBox::error(this, msg1);
	}
}

void EscpWidget::slotReceivedStdout(KProcess*, char *buf, int len)
{
	QString	bufstr = QCString(buf, len);
	m_outbuffer.append(bufstr);
}

void EscpWidget::slotReceivedStderr(KProcess*, char *buf, int len)
{
	QString	bufstr = QCString(buf, len);
	m_errorbuffer.append(bufstr);
}

void EscpWidget::slotButtonClicked()
{
	QString	arg = sender()->name();
	startCommand(arg);
}

void EscpWidget::setDevice(const QString& dev)
{
	m_device = dev;
}

#include "escpwidget.moc"
