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
#include "kdeprintcheck.h"
#include "kmfactory.h"
#include "kmspecialmanager.h"
#include "kxmlcommandselector.h"
#include "kxmlcommand.h"
#include "driver.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicondialog.h>
#include <kfiledialog.h>
#include <kseparator.h>

KMSpecialPrinterDlg::KMSpecialPrinterDlg(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok)
{
	setCaption(i18n("Add Special Printer"));

	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);

	// widget creation
	m_name = new QLineEdit(dummy);
	connect(m_name, SIGNAL(textChanged ( const QString & )),this,SLOT(slotTextChanged(const QString & )));
	m_description = new QLineEdit(dummy);
	m_location = new QLineEdit(dummy);
	QLabel	*m_namelabel = new QLabel(i18n("&Name:"), dummy);
	QLabel	*m_desclabel = new QLabel(i18n("&Description:"), dummy);
	QLabel	*m_loclabel = new QLabel(i18n("&Location:"), dummy);
	m_namelabel->setBuddy(m_name);
	m_desclabel->setBuddy(m_description);
	m_loclabel->setBuddy(m_location);

	KSeparator* sep = new KSeparator( KSeparator::HLine, dummy);

	sep->setFixedHeight(10);
	QGroupBox	*m_gb = new QGroupBox(1, Qt::Horizontal, i18n("Command &Settings"), dummy);
	m_command = new KXmlCommandSelector(true, m_gb, "CommandSelector", this);
	QGroupBox	*m_gb2 = new QGroupBox(0, Qt::Horizontal, i18n("Output File Settings"), dummy);
	m_usefile = new QCheckBox(i18n("Always use &file-extension:"), m_gb2);
	m_extension = new QLineEdit(m_gb2);
	connect(m_usefile, SIGNAL(toggled(bool)), m_extension, SLOT(setEnabled(bool)));
	m_extension->setEnabled(false);

	m_icon = new KIconButton(dummy);
	m_icon->setIcon("fileprint");
	m_icon->setFixedSize(QSize(48,48));

	QWhatsThis::add(m_usefile,
		i18n("<p>The command will use an output file. If checked, make sure the "
		     "command contains an output tag.</p>"));
	QWhatsThis::add(m_command,
		i18n("<p>The command to execute when printing on this special printer. Either enter "
			 "the command to execute directly, or associate/create a command object with/for "
			 "this special printer. The command object is the preferred method as it provides "
			 "support for advanced settings like mime type checking, configurable options and "
			 "requirement list (the plain command is only provided for backward compatibility). "
			 "When using a plain command, the following tags are recognized:</p>"
			 "<ul><li><b>%in</b>: the input file (required).</li>"
			 "<li><b>%out</b>: the output file (required if using an output file).</li>"
			 "<li><b>%psl</b>: the paper size in lower case.</li>"
			 "<li><b>%psu</b>: the paper size with the first letter in upper case.</li></ul>"));
	QWhatsThis::add(m_extension,
		i18n("<p>The default extension for the output file (<u>ex</u>: ps, pdf, ps.gz).</p>"));

	// layout creation
	QVBoxLayout	*l0 = new QVBoxLayout(dummy, 0, 10);
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
	l0->addWidget(m_gb);
	l0->addWidget(m_gb2);
	QHBoxLayout	*l6 = new QHBoxLayout(m_gb2->layout(), 10);
	l6->addWidget(m_usefile, 0);
	l6->addWidget(m_extension, 1);

	enableButton(Ok, !m_name->text().isEmpty());

	// resize dialog
	resize(400,100);
}

void KMSpecialPrinterDlg::slotTextChanged(const QString & text)
{
	enableButton(Ok, !m_name->text().isEmpty());
}

void KMSpecialPrinterDlg::slotOk()
{
	if (!checkSettings())
		return;
	KDialogBase::slotOk();
}

bool KMSpecialPrinterDlg::checkSettings()
{
	QString	msg;
	if (m_name->text().isEmpty())
		msg = i18n("You must provide a non-empty name.");
	else
		KXmlCommandManager::self()->checkCommand(m_command->command(),
					KXmlCommandManager::Basic,
					(m_usefile->isChecked() ? KXmlCommandManager::Basic : KXmlCommandManager::None),
					&msg);

	if (!msg.isEmpty())
		KMessageBox::error(this, i18n("Invalid settings. %1.").arg(msg));

	return (msg.isEmpty());
}

void KMSpecialPrinterDlg::setPrinter(KMPrinter *printer)
{
	if (printer && printer->isSpecial())
	{
		m_command->setCommand(printer->option("kde-special-command"));
		m_usefile->setChecked(printer->option("kde-special-file") == "1");
		m_extension->setText(printer->option("kde-special-extension"));
		m_name->setText(printer->name());
		m_description->setText(printer->description());
		m_location->setText(printer->location());
		m_icon->setIcon(printer->pixmap());

		setCaption(i18n("Configuring %1").arg(printer->name()));
	}
}

KMPrinter* KMSpecialPrinterDlg::printer()
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(m_name->text());
	printer->setPrinterName(m_name->text());
	printer->setPixmap(m_icon->icon());
	printer->setDescription(m_description->text());
	printer->setLocation(m_location->text());
	printer->setOption("kde-special-command",m_command->command());
	printer->setOption("kde-special-file",(m_usefile->isChecked() ? "1" : "0"));
	printer->setOption("kde-special-extension",m_extension->text());
	printer->setType(KMPrinter::Special);
	printer->setState(KMPrinter::Idle);
	return printer;
}

#include "kmspecialprinterdlg.moc"
