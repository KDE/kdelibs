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

#include "kmspecialprinterdlg.h"
#include "kmprinter.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicondialog.h>
#include <kfiledialog.h>
#include <kseparator.h>

KMSpecialPrinterDlg::KMSpecialPrinterDlg(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	setCaption(i18n("Add special printer"));

	// widget creation
	QPushButton	*m_ok = new QPushButton(i18n("OK"), this);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);
	connect(m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	m_ok->setDefault(true);
	QPushButton	*m_browse = new QPushButton(i18n("Browse"), this);
	connect(m_browse,SIGNAL(clicked()),SLOT(slotBrowse()));
	m_name = new QLineEdit(this);
	m_description = new QLineEdit(this);
	m_location = new QLineEdit(this);
	m_command = new QLineEdit(this);
	m_extension = new QLineEdit(this);
	QLabel	*m_namelabel = new QLabel(i18n("Name:"), this);
	QLabel	*m_desclabel = new QLabel(i18n("Description:"), this);
	QLabel	*m_loclabel = new QLabel(i18n("Location:"), this);
	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(10);
	QLabel	*m_cmdlabel = new QLabel(i18n("Command:"), this);
	m_usefile = new QCheckBox(i18n("Use output file with extension:"), this);
	connect(m_usefile, SIGNAL(toggled(bool)), m_extension, SLOT(setEnabled(bool)));
	m_extension->setEnabled(false);
	m_icon = new KIconButton(this);
	m_icon->setIcon("fileprint");
	m_icon->setFixedSize(QSize(48,48));

	QWhatsThis::add(m_usefile, i18n("The command will use an output file."));
	QWhatsThis::add(m_command, i18n("<p>The command to execute when printing on this special printer. "
					"Recognized tags are:</p>"
					"<ul><li><b>%in</b>: the input file (required).</li>"
					"<li><b>%out</b>: the output file (required if using an output file).</li>"
					"<li><b>%psl</b>: the paper size in lower case.</li></ul>"));
	QWhatsThis::add(m_extension, i18n("<p>The default extension for the output file (<u>ex</u>: ps, pdf, ps.gz).</p>"));

	// layout creation
	QVBoxLayout	*l0 = new QVBoxLayout(this, 10, 10);
	QGridLayout	*l1 = new QGridLayout(0, 3, 3, 0, 5);
	l0->addLayout(l1);
	l1->setColStretch(2,1);
	l1->addColSpacing(0,60);
	l1->addMultiCellWidget(m_icon, 0, 2, 0, 0, Qt::AlignCenter);
	l1->addWidget(m_namelabel, 0, 1);
	l1->addWidget(m_desclabel, 1, 1);
	l1->addWidget(m_loclabel, 2, 1);
	l1->addWidget(m_name, 0, 2);
	l1->addWidget(m_description, 1, 2);
	l1->addWidget(m_location, 2, 2);
	l0->addWidget(sep);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l0->addLayout(l2);
	l2->addWidget(m_cmdlabel);
	l2->addWidget(m_command,1);
	l2->addWidget(m_browse);
	QHBoxLayout	*l4 = new QHBoxLayout(0, 0, 10);
	l0->addLayout(l4);
	l4->addWidget(m_usefile, 0);
	l4->addWidget(m_extension, 1);
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 10);
	l0->addSpacing(5);
	l0->addLayout(l3);
	l3->addStretch(1);
	l3->addWidget(m_ok);
	l3->addWidget(m_cancel);

	// resize dialog
	resize(400,100);
}

void KMSpecialPrinterDlg::done(int result)
{
	if (result == QDialog::Accepted)
	{
		if (!checkSettings())
			return;
	}
	KDialog::done(result);
}

bool KMSpecialPrinterDlg::checkSettings()
{
	QString	msg;
	if (m_name->text().isEmpty()) msg = i18n("Empty name.");
	else if (!m_command->text().isEmpty() && m_command->text().find("%in") == -1) msg = i18n("The command doesn't contain the '%1' tag.").arg("%in");
	else if (m_usefile->isChecked())
	{
		if (!m_command->text().isEmpty() && m_command->text().find("%out") == -1)
			msg = i18n("The command doesn't contain the '%1' tag.").arg("%out");
	}
	else if (m_command->text().isEmpty()) msg = i18n("Empty command.");
	if (!msg.isEmpty())
		KMessageBox::error(this,msg);
	return (msg.isEmpty());
}

void KMSpecialPrinterDlg::setPrinter(KMPrinter *printer)
{
	if (printer && printer->isSpecial())
	{
		m_command->setText(printer->option("kde-special-command"));
		m_usefile->setChecked(printer->option("kde-special-file") == "1");
		m_extension->setText(printer->option("kde-special-extension"));
		m_name->setText(printer->name());
		m_description->setText(printer->description());
		m_location->setText(printer->location());
		m_icon->setIcon(printer->pixmap());

		setCaption(i18n("Configuring %1").arg(printer->name()));
	}
}

void KMSpecialPrinterDlg::slotBrowse()
{
	QString	filename = KFileDialog::getOpenFileName(QString::null,QString::null,this);
	if (!filename.isEmpty())
		m_command->setText(filename);
}

KMPrinter* KMSpecialPrinterDlg::printer()
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(m_name->text());
	printer->setPrinterName(m_name->text());
	printer->setPixmap(m_icon->icon());
	printer->setDescription(m_description->text());
	printer->setLocation(m_location->text());
	printer->setOption("kde-special-command",m_command->text());
	printer->setOption("kde-special-file",(m_usefile->isChecked() ? "1" : "0"));
	printer->setOption("kde-special-extension",m_extension->text());
	printer->setType(KMPrinter::Special);
	printer->setState(KMPrinter::Idle);
	return printer;
}

#include "kmspecialprinterdlg.moc"
