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
#include <qcheckbox.h>
#include <qaccel.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kdebug.h>

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
		if (args.count() > 1)
			w->setPrinterName(args[1]);
		dlg->setMainWidget(w);
		return dlg;
	}
};

extern "C"
{
	void* init_kdeprint_tool_escputil()
	{
		return new EscpFactory;
	}
};

EscpWidget::EscpWidget(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	m_hasoutput = false;

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

	QFont	f(font());
	f.setBold(true);
	m_printer = new QLabel(this);
	m_printer->setFont(f);
	m_device = new QLabel(this);
	m_device->setFont(f);
	m_useraw = new QCheckBox(i18n("&Use direct connection (might need root permissions)"), this);

	connect(cleanbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(nozzlebtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(alignbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(inkbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(identbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));

	QLabel	*printerlab = new QLabel(i18n("Printer:"), this);
	printerlab->setAlignment(AlignRight|AlignVCenter);
	QLabel	*devicelab = new QLabel(i18n("Device:"), this);
	devicelab->setAlignment(AlignRight|AlignVCenter);
	QLabel	*cleanlab = new QLabel(i18n("Clea&n print head"), this);
	QLabel	*nozzlelab = new QLabel(i18n("&Print a nozzle test pattern"), this);
	QLabel	*alignlab = new QLabel(i18n("&Align print head"), this);
	QLabel	*inklab = new QLabel(i18n("&Ink level"), this);
	QLabel	*identlab = new QLabel(i18n("P&rinter identification"), this);

	cleanlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	nozzlelab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	alignlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	inklab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	identlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

	cleanbtn->setAccel(QAccel::shortcutKey(cleanlab->text()));
	nozzlebtn->setAccel(QAccel::shortcutKey(nozzlelab->text()));
	alignbtn->setAccel(QAccel::shortcutKey(alignlab->text()));
	inkbtn->setAccel(QAccel::shortcutKey(inklab->text()));
	identbtn->setAccel(QAccel::shortcutKey(identlab->text()));

	KSeparator	*sep = new KSeparator(this);
	sep->setFixedHeight(10);

	QGridLayout	*l0 = new QGridLayout(this, 8, 2, 10, 10);
	QGridLayout	*l1 = new QGridLayout(0, 2, 2, 0, 5);
	l0->addMultiCellLayout(l1, 0, 0, 0, 1);
	l1->addWidget(printerlab, 0, 0);
	l1->addWidget(devicelab, 1, 0);
	l1->addWidget(m_printer, 0, 1);
	l1->addWidget(m_device, 1, 1);
	l1->setColStretch(1, 1);
	l0->addMultiCellWidget(sep, 1, 1, 0, 1);
	l0->addWidget(cleanbtn, 2, 0);
	l0->addWidget(nozzlebtn, 3, 0);
	l0->addWidget(alignbtn, 4, 0);
	l0->addWidget(inkbtn, 5, 0);
	l0->addWidget(identbtn, 6, 0);
	l0->addWidget(cleanlab, 2, 1);
	l0->addWidget(nozzlelab, 3, 1);
	l0->addWidget(alignlab, 4, 1);
	l0->addWidget(inklab, 5, 1);
	l0->addWidget(identlab, 6, 1);
	l0->addMultiCellWidget(m_useraw, 7, 7, 0, 1);
	l0->setColStretch(1, 1);
}

void EscpWidget::startCommand(const QString& arg)
{
	bool	useUSB(false);

	if (m_deviceURL.isEmpty())
	{
		KMessageBox::error(this, i18n("Internal error: no device set."));
		return;
	}
	else
	{
		QString	protocol = m_deviceURL.protocol();
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
		KMessageBox::error(this, i18n("An escputil process is still running. "
		                              "You must wait until its completion before continuing."));
		return;
	}

	QString	exestr = KStandardDirs::findExe("escputil");
	if (exestr.isEmpty())
	{
		KMessageBox::error(this, i18n("The executable escputil cannot be found in your "
		                              "PATH environment variable. Make sure gimp-print is "
		                              "installed and that escputil is in your PATH."));
		return;
	}

	m_proc.clearArguments();
	m_proc << exestr;
	if (m_useraw->isChecked() || arg == "-i")
		m_proc << "-r" << m_deviceURL.path();
	else
		m_proc << "-P" << m_printer->text();
	if (useUSB)
		m_proc << "-u";

	m_proc << arg << "-q";
	m_errorbuffer = m_outbuffer = QString::null;
	m_hasoutput = ( arg == "-i" || arg == "-d" );
	for ( QValueList<QCString>::ConstIterator it=m_proc.args().begin(); it!=m_proc.args().end(); ++it )
		kdDebug() << "ARG: " << *it << endl;
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
		QString	msg1 = "<qt>"+i18n("Operation terminated with errors.")+"</qt>";
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
	else if ( !m_outbuffer.isEmpty() && m_hasoutput )
	{
		KMessageBox::information( this, m_outbuffer );
	}
	m_hasoutput = false;
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

void EscpWidget::setPrinterName(const QString& p)
{
	m_printer->setText(p);
}

void EscpWidget::setDevice(const QString& dev)
{
	m_deviceURL = dev;
	m_device->setText(dev);
}

#include "escpwidget.moc"
