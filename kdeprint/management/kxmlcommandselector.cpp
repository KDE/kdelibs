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

#include "kxmlcommandselector.h"
#include "kxmlcommand.h"
#include "kxmlcommanddlg.h"

#include <qcombobox.h>
#include <kpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kseparator.h>
#include <kguiitem.h>

KXmlCommandSelector::KXmlCommandSelector(bool canBeNull, QWidget *parent, const char *name)
: QWidget(parent, name)
{
	m_cmd = new QComboBox(this);
	connect(m_cmd, SIGNAL(activated(int)), SLOT(slotCommandSelected(int)));
	QPushButton	*m_add = new KPushButton(this);
	QPushButton	*m_edit = new KPushButton(this);
	m_add->setPixmap(SmallIcon("filenew"));
	m_edit->setPixmap(SmallIcon("configure"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddCommand()));
	connect(m_edit, SIGNAL(clicked()), SLOT(slotEditCommand()));
	QToolTip::add(m_add, i18n("New Command"));
	QToolTip::add(m_edit, i18n("Edit Command"));
	m_shortinfo = new QLabel(this);

	m_line = 0;
	m_usefilter = 0;
	QPushButton	*m_browse = 0;

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 10);

	if (canBeNull)
	{
		m_line = new QLineEdit(this);
		m_browse = new KPushButton(KGuiItem(i18n("&Browse..."), "fileopen"), this);
		m_usefilter = new QCheckBox(i18n("Use co&mmand:"), this);
		connect(m_browse, SIGNAL(clicked()), SLOT(slotBrowse()));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_line, SLOT(setDisabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_browse, SLOT(setDisabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_cmd, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_add, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_edit, SLOT(setEnabled(bool)));
		connect(m_usefilter, SIGNAL(toggled(bool)), m_shortinfo, SLOT(setEnabled(bool)));
		m_usefilter->setChecked(true);
		m_usefilter->setChecked(false);
		//setFocusProxy(m_line);
		setTabOrder(m_usefilter, m_cmd);
		setTabOrder(m_cmd, m_add);
		setTabOrder(m_add, m_edit);

		QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 10);
		l0->addLayout(l1);
		l1->addWidget(m_line);
		l1->addWidget(m_browse);

		KSeparator	*sep = new KSeparator(Qt::Horizontal, this);
		l0->addWidget(sep);
	}
	else
		setFocusProxy(m_cmd);

	QGridLayout	*l2 = new QGridLayout(0, 2, (m_usefilter?3:2), 0, 5);
	int	c(0);
	l0->addLayout(l2);
	if (m_usefilter)
	{
		l2->addWidget(m_usefilter, 0, c++);
	}
	l2->addWidget(m_cmd, 0, c);
	l2->addWidget(m_shortinfo, 1, c);
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 0);
	l2->addLayout(l3, 0, c+1);
	l3->addWidget(m_add);
	l3->addWidget(m_edit);

	loadCommands();
}

void KXmlCommandSelector::loadCommands()
{
	QString	thisCmd = (m_cmd->currentItem() != -1 ? m_cmdlist[m_cmd->currentItem()] : QString::null);

	m_cmd->clear();
	m_cmdlist.clear();

	QStringList	list = KXmlCommandManager::self()->commandListWithDescription();
	QStringList	desclist;
	for (QStringList::Iterator it=list.begin(); it!=list.end(); ++it)
	{
		m_cmdlist << (*it);
		++it;
		desclist << (*it);
	}
	m_cmd->insertStringList(desclist);

	int	index = m_cmdlist.findIndex(thisCmd);
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentItem() != -1)
		slotCommandSelected(m_cmd->currentItem());
}

QString KXmlCommandSelector::command() const
{
	QString	cmd;
	if (m_line && !m_usefilter->isChecked())
		cmd = m_line->text();
	else
		cmd = m_cmdlist[m_cmd->currentItem()];
	return cmd;
}

void KXmlCommandSelector::setCommand(const QString& cmd)
{
	int	index = m_cmdlist.findIndex(cmd);;

	if (m_usefilter)
		m_usefilter->setChecked(index != -1);
	if (m_line)
		m_line->setText((index == -1 ? cmd : QString::null));
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentItem() != -1)
		slotCommandSelected(m_cmd->currentItem());
}

void KXmlCommandSelector::slotAddCommand()
{
	bool	ok(false);
	QString	cmdId = KLineEditDlg::getText(i18n("Command Name"), i18n("Enter an identification name for the new command:"), QString::null, &ok, this);
	if (ok)
	{
		bool	added(true);

		if (m_cmdlist.findIndex(cmdId) != -1)
		{
			if (KMessageBox::warningContinueCancel(
				this,
				i18n("A command named %1 already exists. Do you want "
				     "to continue and edit the existing one?").arg(cmdId),
				QString::null,
				i18n("Continue")) == KMessageBox::Cancel)
			{
				return;
			}
			else
				added = false;
	}

		KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(cmdId);
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
			KXmlCommandManager::self()->saveCommand(xmlCmd);

		if (added)
			loadCommands();
	}
}

void KXmlCommandSelector::slotEditCommand()
{
	QString	xmlId = m_cmdlist[m_cmd->currentItem()];
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(xmlId);
	if (xmlCmd)
	{
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
		{
			// force to load the driver if not already done
			xmlCmd->driver();
			KXmlCommandManager::self()->saveCommand(xmlCmd);
		}
		m_cmd->changeItem(xmlCmd->description(), m_cmd->currentItem());
		delete xmlCmd;
		slotCommandSelected(m_cmd->currentItem());
	}
	else
		KMessageBox::error(this, i18n("Internal Error. The XML driver for the command %1 could not be found.").arg(xmlId));
}

void KXmlCommandSelector::slotBrowse()
{
	QString	filename = KFileDialog::getOpenFileName(QString::null, QString::null, this);
	if (!filename.isEmpty() && m_line)
		m_line->setText(filename);
}

void KXmlCommandSelector::slotCommandSelected(int ID)
{
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(m_cmdlist[ID]);
	if (xmlCmd)
	{
		QString	msg(QString::fromLocal8Bit("(ID = %1, %2 = ").arg(xmlCmd->name()).arg(i18n("output")));
		if (KXmlCommandManager::self()->checkCommand(xmlCmd->name(), KXmlCommandManager::None, KXmlCommandManager::Basic))
		{
			if (xmlCmd->mimeType() == "all/all")
				msg.append(i18n("undefined"));
			else
				msg.append(xmlCmd->mimeType());
		}
		else
			msg.append(i18n("not allowed"));
		msg.append(")");
		m_shortinfo->setText(msg);
	}
}

#include "kxmlcommandselector.moc"
