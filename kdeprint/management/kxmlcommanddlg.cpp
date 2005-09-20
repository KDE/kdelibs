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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kxmlcommanddlg.h"
#include "driver.h"
#include "kxmlcommand.h"

#include <qlayout.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <q3groupbox.h>
#include <q3widgetstack.h>
#include <qtoolbutton.h>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <ktextedit.h>
#include <qregexp.h>
#include <qapplication.h>

#include <klistview.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kseparator.h>
#include <klistbox.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <krandom.h>
#include <kdebug.h>
#include <kguiitem.h>

QString generateId(const QMap<QString, DrBase*>& map)
{
	int	index(-1);
	while (map.contains(QString::fromLatin1("item%1").arg(++index))) ;
	return QString::fromLatin1("item%1").arg(index);
}

Q3ListViewItem* findPrev(Q3ListViewItem *item)
{
	Q3ListViewItem	*prev = item->itemAbove();
	while (prev && prev->depth() > item->depth())
		prev = prev->itemAbove();
	if (prev && prev->depth() == item->depth())
		return prev;
	else
		return 0;
}

Q3ListViewItem* findNext(Q3ListViewItem *item)
{
	Q3ListViewItem	*next = item->itemBelow();
	while (next && next->depth() > item->depth())
		next = next->itemBelow();
	if (next && next->depth() == item->depth())
		return next;
	else
		return 0;
}

KXmlCommandAdvancedDlg::KXmlCommandAdvancedDlg( QWidget *parent )
    : QWidget(parent)
{
	m_xmlcmd = 0;

	m_command = new QLineEdit(this);
	m_view = new KListView(this);
	m_view->addColumn("");
	m_view->header()->hide();
	m_view->setSorting(-1);
	m_apply = new QToolButton(this);
	m_apply->setIcon( QApplication::reverseLayout()? SmallIconSet( "forward" ) : SmallIconSet("back"));
	m_addgrp = new QToolButton(this);
	m_addgrp->setIcon(SmallIconSet("folder"));
	m_addopt = new QToolButton(this);
	m_addopt->setIcon(SmallIconSet("document"));
	m_delopt = new QToolButton(this);
	m_delopt->setIcon(SmallIconSet("editdelete"));
	m_up = new QToolButton(this);
	m_up->setIcon(SmallIconSet("up"));
	m_down = new QToolButton(this);
	m_down->setIcon(SmallIconSet("down"));
	m_dummy = new QWidget(this);
	m_desc = new QLineEdit(m_dummy);
	m_name = new QLineEdit(m_dummy);
	m_type = new QComboBox(m_dummy);
	m_type->insertItem(i18n("String"));
	m_type->insertItem(i18n("Integer"));
	m_type->insertItem(i18n("Float"));
	m_type->insertItem(i18n("List"));
	m_type->insertItem(i18n("Boolean"));
	m_format = new QLineEdit(m_dummy);
	m_default = new QLineEdit(m_dummy);
	QLabel	*m_namelab = new QLabel(i18n("&Name:"), m_dummy);
	QLabel	*m_desclab = new QLabel(i18n("&Description:"), m_dummy);
	QLabel	*m_formatlab = new QLabel(i18n("&Format:"), m_dummy);
	QLabel	*m_typelab = new QLabel(i18n("&Type:"), m_dummy);
	QLabel	*m_defaultlab = new QLabel(i18n("Default &value:"), m_dummy);
	QLabel	*m_commandlab = new QLabel(i18n("Co&mmand:"), this);
	m_namelab->setBuddy(m_name);
	m_desclab->setBuddy(m_desc);
	m_formatlab->setBuddy(m_format);
	m_typelab->setBuddy(m_type);
	m_defaultlab->setBuddy(m_default);
	m_commandlab->setBuddy(m_command);
	m_persistent = new QCheckBox( i18n( "&Persistent option" ), m_dummy );

	Q3GroupBox	*gb = new Q3GroupBox(0, Qt::Horizontal, i18n("Va&lues"), m_dummy);
	m_stack = new Q3WidgetStack(gb);
	QWidget	*w1 = new QWidget(m_stack), *w2 = new QWidget(m_stack), *w3 = new QWidget(m_stack);
	m_stack->addWidget(w1, 1);
	m_stack->addWidget(w2, 2);
	m_stack->addWidget(w3, 3);
	m_edit1 = new QLineEdit(w1);
	m_edit2 = new QLineEdit(w1);
	QLabel	*m_editlab1 = new QLabel(i18n("Minimum v&alue:"), w1);
	QLabel	*m_editlab2 = new QLabel(i18n("Ma&ximum value:"), w1);
	m_editlab1->setBuddy(m_edit1);
	m_editlab2->setBuddy(m_edit2);
	m_values = new KListView(w2);
	m_values->addColumn(i18n("Name"));
	m_values->addColumn(i18n("Description"));
	m_values->setAllColumnsShowFocus(true);
	m_values->setSorting(-1);
	m_values->setMaximumHeight(110);
	m_addval = new QToolButton(w2);
	m_addval->setIcon(SmallIconSet("editcopy"));
	m_delval = new QToolButton(w2);
	m_delval->setIcon(SmallIconSet("editdelete"));
	m_addval->setToolTip(i18n("Add value"));
	m_delval->setToolTip(i18n("Delete value"));

	m_apply->setToolTip(i18n("Apply changes"));
	m_addgrp->setToolTip(i18n("Add group"));
	m_addopt->setToolTip(i18n("Add option"));
	m_delopt->setToolTip(i18n("Delete item"));
	m_up->setToolTip(i18n("Move up"));
	m_down->setToolTip(i18n("Move down"));

	KSeparator	*sep1 = new KSeparator(KSeparator::HLine, m_dummy);

	Q3GroupBox	*gb_input = new Q3GroupBox(0, Qt::Horizontal, i18n("&Input From"), this);
	Q3GroupBox	*gb_output = new Q3GroupBox(0, Qt::Horizontal, i18n("O&utput To"), this);
	QLabel	*m_inputfilelab = new QLabel(i18n("File:"), gb_input);
	QLabel	*m_inputpipelab = new QLabel(i18n("Pipe:"), gb_input);
	QLabel	*m_outputfilelab = new QLabel(i18n("File:"), gb_output);
	QLabel	*m_outputpipelab = new QLabel(i18n("Pipe:"), gb_output);
	m_inputfile = new QLineEdit(gb_input);
	m_inputpipe = new QLineEdit(gb_input);
	m_outputfile = new QLineEdit(gb_output);
	m_outputpipe = new QLineEdit(gb_output);

	m_comment = new KTextEdit( this );
	m_comment->setTextFormat( Qt::PlainText );
	QLabel *m_commentlab = new QLabel( i18n( "Comment:" ), this );

	QVBoxLayout	*l2 = new QVBoxLayout(this, 0, KDialog::spacingHint());
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, KDialog::spacingHint());
	QVBoxLayout	*l7 = new QVBoxLayout(0, 0, 0);
	l2->addLayout(l3, 0);
	l3->addWidget(m_commandlab);
	l3->addWidget(m_command);
	QHBoxLayout	*l0 = new QHBoxLayout(0, 0, KDialog::spacingHint());
	QGridLayout	*l10 = new QGridLayout(0, 2, 2, 0, KDialog::spacingHint());
	l2->addLayout(l0, 1);
	l0->addLayout(l10);
	l10->addMultiCellWidget(m_view, 0, 0, 0, 1);
	l10->addWidget(gb_input, 1, 0);
	l10->addWidget(gb_output, 1, 1);
	l10->setRowStretch(0, 1);
	l0->addLayout(l7);
	l7->addWidget(m_apply);
	l7->addSpacing(5);
	l7->addWidget(m_addgrp);
	l7->addWidget(m_addopt);
	l7->addWidget(m_delopt);
	l7->addSpacing(5);
	l7->addWidget(m_up);
	l7->addWidget(m_down);
	l7->addStretch(1);
	l0->addWidget(m_dummy, 1);
	QGridLayout	*l1 = new QGridLayout(m_dummy, 9, 2, 0, KDialog::spacingHint());
	l1->addWidget(m_desclab, 0, 0, Qt::AlignRight|Qt::AlignVCenter);
	l1->addWidget(m_desc, 0, 1);
	l1->addMultiCellWidget(sep1, 1, 1, 0, 1);
	l1->addWidget(m_namelab, 2, 0, Qt::AlignRight|Qt::AlignVCenter);
	l1->addWidget(m_name, 2, 1);
	l1->addWidget(m_typelab, 3, 0, Qt::AlignRight|Qt::AlignVCenter);
	l1->addWidget(m_type, 3, 1);
	l1->addWidget(m_formatlab, 4, 0, Qt::AlignRight|Qt::AlignVCenter);
	l1->addWidget(m_format, 4, 1);
	l1->addWidget(m_defaultlab, 5, 0, Qt::AlignRight|Qt::AlignVCenter);
	l1->addWidget(m_default, 5, 1);
	l1->addWidget( m_persistent, 6, 1 );
	l1->addMultiCellWidget(gb, 7, 7, 0, 1);
	l1->setRowStretch(8, 1);

	QHBoxLayout	*l4 = new QHBoxLayout(w2, 0, KDialog::spacingHint());
	l4->addWidget(m_values);
	QVBoxLayout	*l6 = new QVBoxLayout(0, 0, 0);
	l4->addLayout(l6);
	l6->addWidget(m_addval);
	l6->addWidget(m_delval);
	l6->addStretch(1);
	QGridLayout	*l5 = new QGridLayout(w1, 3, 2, 0, KDialog::spacingHint());
	l5->setRowStretch(2, 1);
	l5->addWidget(m_editlab1, 0, 0, Qt::AlignRight|Qt::AlignVCenter);
	l5->addWidget(m_editlab2, 1, 0, Qt::AlignRight|Qt::AlignVCenter);
	l5->addWidget(m_edit1, 0, 1);
	l5->addWidget(m_edit2, 1, 1);

	QGridLayout	*l8 = new QGridLayout(gb_input->layout(), 2, 2,
		KDialog::spacingHint());
	QGridLayout	*l9 = new QGridLayout(gb_output->layout(), 2, 2,
		KDialog::spacingHint());
	l8->addWidget(m_inputfilelab, 0, 0);
	l8->addWidget(m_inputpipelab, 1, 0);
	l8->addWidget(m_inputfile, 0, 1);
	l8->addWidget(m_inputpipe, 1, 1);
	l9->addWidget(m_outputfilelab, 0, 0);
	l9->addWidget(m_outputpipelab, 1, 0);
	l9->addWidget(m_outputfile, 0, 1);
	l9->addWidget(m_outputpipe, 1, 1);

	QVBoxLayout	*l11 = new QVBoxLayout(gb->layout());
	l11->addWidget(m_stack);

	QVBoxLayout *l12 = new QVBoxLayout( 0, 0, 0 );
	l2->addSpacing( 10 );
	l2->addLayout( l12 );
	l12->addWidget( m_commentlab );
	l12->addWidget( m_comment );

	connect(m_view, SIGNAL(selectionChanged(Q3ListViewItem*)), SLOT(slotSelectionChanged(Q3ListViewItem*)));
	connect(m_values, SIGNAL(selectionChanged(Q3ListViewItem*)), SLOT(slotValueSelected(Q3ListViewItem*)));
	connect(m_type, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)));
	connect(m_addval, SIGNAL(clicked()), SLOT(slotAddValue()));
	connect(m_delval, SIGNAL(clicked()), SLOT(slotRemoveValue()));
	connect(m_apply, SIGNAL(clicked()), SLOT(slotApplyChanges()));
	connect(m_addgrp, SIGNAL(clicked()), SLOT(slotAddGroup()));
	connect(m_addopt, SIGNAL(clicked()), SLOT(slotAddOption()));
	connect(m_delopt, SIGNAL(clicked()), SLOT(slotRemoveItem()));
	connect(m_up, SIGNAL(clicked()), SLOT(slotMoveUp()));
	connect(m_down, SIGNAL(clicked()), SLOT(slotMoveDown()));
	connect(m_command, SIGNAL(textChanged(const QString&)), SLOT(slotCommandChanged(const QString&)));
	connect(m_view, SIGNAL(itemRenamed(Q3ListViewItem*,int)), SLOT(slotOptionRenamed(Q3ListViewItem*,int)));
	connect(m_desc, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_name, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_format, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_default, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_edit1, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_edit2, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(m_type, SIGNAL(activated(int)), SLOT(slotChanged()));
	connect(m_addval, SIGNAL(clicked()), SLOT(slotChanged()));
	connect(m_delval, SIGNAL(clicked()), SLOT(slotChanged()));
	connect( m_persistent, SIGNAL( toggled(bool) ), SLOT( slotChanged() ) );
	m_dummy->setEnabled(false);
	viewItem(0);

	/**
	 * Add some short help for the normal (non expert) user
	 */
	m_name->setWhatsThis(i18n(
				"An identification string. Use only alphanumeric characters except spaces. "
				"The string <b>__root__</b> is reserved for internal use." ) );
	m_namelab->setWhatsThis(m_name->whatsThis() );
	m_desc->setWhatsThis(i18n(
				"A description string. This string is shown in the interface, and should "
				"be explicit enough about the role of the corresponding option." ) );
	m_desclab->setWhatsThis(m_desc->whatsThis() );
	m_type->setWhatsThis(i18n(
				"The type of the option. This determines how the option is presented "
				"graphically to the user." ) );
	m_typelab->setWhatsThis(m_type->whatsThis() );
	m_format->setWhatsThis(i18n(
				"The format of the option. This determines how the option is formatted "
				"for inclusion in the global command line. The tag <b>%value</b> can be used "
				"to represent the user selection. This tag will be replaced at run-time by a "
				"string representation of the option value." ) );
	m_formatlab->setWhatsThis(m_format->whatsThis() );
	m_default->setWhatsThis(i18n(
				"The default value of the option. For non persistent options, nothing is "
				"added to the command line if the option has that default value. If this "
				"value does not correspond to the actual default value of the underlying "
				"utility, make the option persistent to avoid unwanted effects." ) );
	m_defaultlab->setWhatsThis(m_default->whatsThis() );
	m_persistent->setWhatsThis(i18n(
				"Make the option persistent. A persistent option is always written to the "
				"command line, whatever its value. This is useful when the chosen default "
				"value does not match with the actual default value of the underlying utility." ) );
	m_command->setWhatsThis(i18n(
				"The full command line to execute the associated underlying utility. This "
				"command line is based on a mechanism of tags that are replaced at run-time. "
				"The supported tags are:<ul>"
				"<li><b>%filterargs</b>: command options</li>"
				"<li><b>%filterinput</b>: input specification</li>"
				"<li><b>%filteroutput</b>: output specification</li>"
				"<li><b>%psu</b>: the page size in upper case</li>"
				"<li><b>%psl</b>: the page size in lower case</li></ul>" ) );
	m_commandlab->setWhatsThis(m_command->whatsThis() );
	m_inputfile->setWhatsThis(i18n(
				"Input specification when the underlying utility reads input data from a file. Use "
				"the tag <b>%in</b> to represent the input filename." ) );
	m_inputfilelab->setWhatsThis(m_inputfile->whatsThis() );
	m_outputfile->setWhatsThis(i18n(
				"Output specification when the underlying utility writes output data to a file. Use "
				"the tag <b>%out</b> to represent the output filename." ) );
	m_outputfilelab->setWhatsThis(m_outputfile->whatsThis() );
	m_inputpipe->setWhatsThis(i18n(
				"Input specification when the underlying utility reads input data from its "
				"standard input." ) );
	m_inputpipelab->setWhatsThis(m_inputpipe->whatsThis() );
	m_outputpipe->setWhatsThis(i18n(
				"Output specification when the underlying utility writes output data to its "
				"standard output." ) );
	m_outputpipelab->setWhatsThis(m_outputpipe->whatsThis() );
	m_comment->setWhatsThis(i18n(
				"A comment about the underlying utility, which can be viewed by the user "
				"from the interface. This comment string supports basic HTML tags like "
				"&lt;a&gt;, &lt;b&gt; or &lt;i&gt;." ) );
	m_commentlab->setWhatsThis(m_comment->whatsThis() );

	resize(660, 200);
}

KXmlCommandAdvancedDlg::~KXmlCommandAdvancedDlg()
{
	if (m_opts.count() > 0)
	{
		kdDebug() << "KXmlCommandAdvancedDlg: " << m_opts.count() << " items remaining" << endl;
		for (QMap<QString,DrBase*>::ConstIterator it=m_opts.begin(); it!=m_opts.end(); ++it)
		{
			//kdDebug() << "Item: name=" << (*it)->name() << endl;
			delete (*it);
		}
	}
}

void KXmlCommandAdvancedDlg::setCommand(KXmlCommand *xmlcmd)
{
	m_xmlcmd = xmlcmd;
	if (m_xmlcmd)
		parseXmlCommand(m_xmlcmd);
}

void KXmlCommandAdvancedDlg::parseXmlCommand(KXmlCommand *xmlcmd)
{
	m_view->clear();
	Q3ListViewItem	*root = new Q3ListViewItem(m_view, xmlcmd->name(), xmlcmd->name());
	DrMain	*driver = xmlcmd->driver();

	root->setPixmap(0, SmallIcon("fileprint"));
	root->setOpen(true);
	if (driver)
	{
		DrMain	*clone = driver->cloneDriver();
		if (!clone->get("text").isEmpty())
			root->setText(0, clone->get("text"));
		root->setText(1, "__root__");
		clone->setName("__root__");
		m_opts["__root__"] = clone;
		parseGroupItem(clone, root);
		clone->flatten();
	}
	m_command->setText(xmlcmd->command());
	m_inputfile->setText(xmlcmd->io(true, false));
	m_inputpipe->setText(xmlcmd->io(true, true));
	m_outputfile->setText(xmlcmd->io(false, false));
	m_outputpipe->setText(xmlcmd->io(false, true));
	m_comment->setText( xmlcmd->comment() );

	viewItem(0);
}

void KXmlCommandAdvancedDlg::parseGroupItem(DrGroup *grp, Q3ListViewItem *parent)
{
	Q3ListViewItem	*item(0);

	Q3PtrListIterator<DrGroup>	git(grp->groups());
	for (; git.current(); ++git)
	{
		QString	namestr = git.current()->name();
		if (namestr.isEmpty())
		{
			namestr = "group_"+KRandom::randomString(4);
		}
		git.current()->setName(namestr);
		item = new Q3ListViewItem(parent, item, git.current()->get("text"), git.current()->name());
		item->setPixmap(0, SmallIcon("folder"));
		item->setOpen(true);
		item->setRenameEnabled(0, true);
		parseGroupItem(git.current(), item);
		m_opts[namestr] = git.current();
	}

	Q3PtrListIterator<DrBase>	oit(grp->options());
	for (; oit.current(); ++oit)
	{
		QString	namestr = oit.current()->name().mid(m_xmlcmd->name().length()+6);
		if (namestr.isEmpty())
		{
			namestr = "option_"+KRandom::randomString(4);
		}
		oit.current()->setName(namestr);
		item = new Q3ListViewItem(parent, item, oit.current()->get("text"), namestr);
		item->setPixmap(0, SmallIcon("document"));
		item->setRenameEnabled(0, true);
		m_opts[namestr] = oit.current();
	}
}

void KXmlCommandAdvancedDlg::slotSelectionChanged(Q3ListViewItem *item)
{
	if (item && item->depth() == 0)
		item = 0;
	viewItem(item);
}

void KXmlCommandAdvancedDlg::viewItem(Q3ListViewItem *item)
{
	m_dummy->setEnabled((item != 0));
	m_name->setText("");
	m_desc->setText("");
	m_format->setText("");
	m_default->setText("");
	m_values->clear();
	m_edit1->setText("");
	m_edit2->setText("");
	m_persistent->setChecked( false );
	int	typeId(-1);
	if (item)
	{
		m_name->setText(item->text(1));
		m_desc->setText(item->text(0));

		DrBase	*opt = (m_opts.contains(item->text(1)) ? m_opts[item->text(1)] : 0);
		if (opt)
		{
			bool	isgroup = (opt->type() < DrBase::String);
			if (!isgroup)
			{
				m_type->setCurrentItem(opt->type() - DrBase::String);
				typeId = m_type->currentItem();
				m_format->setText(opt->get("format"));
				m_default->setText(opt->get("default"));
			}
			m_type->setEnabled(!isgroup);
			m_default->setEnabled(!isgroup);
			m_format->setEnabled(!isgroup);
			m_stack->setEnabled(!isgroup);

			switch (opt->type())
			{
				case DrBase::Float:
				case DrBase::Integer:
					m_edit1->setText(opt->get("minval"));
					m_edit2->setText(opt->get("maxval"));
					break;
				case DrBase::Boolean:
				case DrBase::List:
					{
						Q3PtrListIterator<DrBase>	it(*(static_cast<DrListOption*>(opt)->choices()));
						Q3ListViewItem	*item(0);
						for (; it.current(); ++it)
						{
							item = new Q3ListViewItem(m_values, item, it.current()->name(), it.current()->get("text"));
							item->setRenameEnabled(0, true);
							item->setRenameEnabled(1, true);
						}
						break;
					}
				default:
					break;
			}

			m_addgrp->setEnabled(isgroup);
			m_addopt->setEnabled(isgroup);

			Q3ListViewItem	*prevItem = findPrev(item), *nextItem = findNext(item);
			DrBase	*prevOpt = (prevItem && m_opts.contains(prevItem->text(1)) ? m_opts[prevItem->text(1)] : 0);
			DrBase	*nextOpt = (nextItem && m_opts.contains(nextItem->text(1)) ? m_opts[nextItem->text(1)] : 0);
			m_up->setEnabled(prevOpt && !(prevOpt->type() < DrBase::String && opt->type() >= DrBase::String));
			m_down->setEnabled(nextOpt && !(isgroup && nextOpt->type() >= DrBase::String));

			m_persistent->setChecked( opt->get( "persistent" ) == "1" );
		}

		m_delopt->setEnabled(true);
		m_dummy->setEnabled(opt);
	}
	else
	{
		m_delopt->setEnabled(false);
		m_addopt->setEnabled(m_view->currentItem() && m_view->isEnabled());
		m_addgrp->setEnabled(m_view->currentItem() && m_view->isEnabled());
		m_up->setEnabled(false);
		m_down->setEnabled(false);
	}
	slotTypeChanged(typeId);
	m_apply->setEnabled(false);
}

void KXmlCommandAdvancedDlg::slotTypeChanged(int ID)
{
	int	wId(3);
	ID += DrBase::String;
	switch (ID)
	{
		case DrBase::Float:
		case DrBase::Integer:
			wId = 1;
			break;
		case DrBase::Boolean:
		case DrBase::List:
			wId = 2;
			slotValueSelected(m_values->currentItem());
			break;
	}
	m_stack->raiseWidget(wId);
}

void KXmlCommandAdvancedDlg::slotAddValue()
{
	Q3ListViewItem	*item = new Q3ListViewItem(m_values, m_values->lastItem(), i18n("Name"), i18n("Description"));
	item->setRenameEnabled(0, true);
	item->setRenameEnabled(1, true);
	m_values->ensureItemVisible(item);
	slotValueSelected(item);
	item->startRename(0);
}

void KXmlCommandAdvancedDlg::slotRemoveValue()
{
	Q3ListViewItem	*item = m_values->currentItem();
	if (item)
		delete item;
	slotValueSelected(m_values->currentItem());
}

void KXmlCommandAdvancedDlg::slotApplyChanges()
{
	Q3ListViewItem	*item = m_view->currentItem();
	if (item)
	{
		if (m_name->text().isEmpty() || m_name->text() == "__root__")
		{
			KMessageBox::error(this, i18n("Invalid identification name. Empty strings and \"__root__\" are not allowed."));
			return;
		}

		m_apply->setEnabled(false);

		DrBase	*opt = (m_opts.contains(item->text(1)) ? m_opts[item->text(1)] : 0);
		m_opts.remove(item->text(1));
		delete opt;

		// update tree item
		item->setText(0, m_desc->text());
		item->setText(1, m_name->text());

		// recreate option
		if (m_type->isEnabled())
		{
			int	type = m_type->currentItem() + DrBase::String;
			switch (type)
			{
				case DrBase::Integer:
				case DrBase::Float:
					if (type == DrBase::Integer)
						opt = new DrIntegerOption;
					else
						opt = new DrFloatOption;
					opt->set("minval", m_edit1->text());
					opt->set("maxval", m_edit2->text());
					break;
				case DrBase::List:
				case DrBase::Boolean:
					{
						if (type == DrBase::List)
							opt = new DrListOption;
						else
							opt = new DrBooleanOption;
						DrListOption	*lopt = static_cast<DrListOption*>(opt);
						Q3ListViewItem	*item = m_values->firstChild();
						while (item)
						{
							DrBase	*choice = new DrBase;
							choice->setName(item->text(0));
							choice->set("text", item->text(1));
							lopt->addChoice(choice);
							item = item->nextSibling();
						}
						break;
					}
				case DrBase::String:
					opt = new DrStringOption;
					break;

			}
			opt->set("format", m_format->text());
			opt->set("default", m_default->text());
			opt->setValueText(opt->get("default"));
		}
		else
			opt = new DrGroup;

		opt->setName((m_name->text().isEmpty() ? generateId(m_opts) : m_name->text()));
		opt->set("text", m_desc->text());
		opt->set( "persistent", m_persistent->isChecked() ? "1" : "0" );

		m_opts[opt->name()] = opt;
	}
}

void KXmlCommandAdvancedDlg::slotChanged()
{
	m_apply->setEnabled(true);
}

void KXmlCommandAdvancedDlg::slotAddGroup()
{
	if (m_view->currentItem())
	{
		QString	ID = generateId(m_opts);

		DrGroup	*grp = new DrGroup;
		grp->setName(ID);
		grp->set("text", i18n("New Group"));
		m_opts[ID] = grp;

		Q3ListViewItem	*item = new Q3ListViewItem(m_view->currentItem(), i18n("New Group"), ID);
		item->setRenameEnabled(0, true);
		item->setPixmap(0, SmallIcon("folder"));
		m_view->ensureItemVisible(item);
		item->startRename(0);
	}
}

void KXmlCommandAdvancedDlg::slotAddOption()
{
	if (m_view->currentItem())
	{
		QString	ID = generateId(m_opts);

		DrBase	*opt = new DrStringOption;
		opt->setName(ID);
		opt->set("text", i18n("New Option"));
		m_opts[ID] = opt;

		Q3ListViewItem	*item = new Q3ListViewItem(m_view->currentItem(), i18n("New Option"), ID);
		item->setRenameEnabled(0, true);
		item->setPixmap(0, SmallIcon("document"));
		m_view->ensureItemVisible(item);
		item->startRename(0);
	}
}

void KXmlCommandAdvancedDlg::slotRemoveItem()
{
	Q3ListViewItem	*item = m_view->currentItem();
	if (item)
	{
		Q3ListViewItem	*newCurrent(item->nextSibling());
		if (!newCurrent)
			newCurrent = item->parent();
		removeItem(item);
		delete item;
		m_view->setSelected(newCurrent, true);
	}
}

void KXmlCommandAdvancedDlg::removeItem(Q3ListViewItem *item)
{
	delete m_opts[item->text(1)];
	m_opts.remove(item->text(1));
	Q3ListViewItem	*child = item->firstChild();
	while (child && item)
	{
		removeItem(child);
                if ( item )
                    item = item->nextSibling();
	}
}

void KXmlCommandAdvancedDlg::slotMoveUp()
{
	Q3ListViewItem	*item = m_view->currentItem(), *prev = 0;
	if (item && (prev=findPrev(item)))
	{
		Q3ListViewItem	*after(0);
		if ((after=findPrev(prev)) != 0)
			item->moveItem(after);
		else
		{
			Q3ListViewItem	*parent = item->parent();
			parent->takeItem(item);
			parent->insertItem(item);
		}
		m_view->setSelected(item, true);
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::slotMoveDown()
{
	Q3ListViewItem	*item = m_view->currentItem(), *next = 0;
	if (item && (next=findNext(item)))
	{
		item->moveItem(next);
		m_view->setSelected(item, true);
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::slotCommandChanged(const QString& cmd)
{
	m_inputfile->parentWidget()->setEnabled(cmd.find("%filterinput") != -1);
	m_outputfile->parentWidget()->setEnabled(cmd.find("%filteroutput") != -1);
	m_view->setEnabled(cmd.find("%filterargs") != -1);
	m_name->parentWidget()->setEnabled(m_view->isEnabled());
	slotSelectionChanged((m_view->isEnabled() ? m_view->currentItem() : 0));
	m_view->setOpen(m_view->firstChild(), m_view->isEnabled());
}

void KXmlCommandAdvancedDlg::slotValueSelected(Q3ListViewItem *item)
{
	m_addval->setEnabled(m_type->currentItem() != 4 || m_values->childCount() < 2);
	m_delval->setEnabled(item != 0);
}

void KXmlCommandAdvancedDlg::slotOptionRenamed(Q3ListViewItem *item, int)
{
	if (item && m_opts.contains(item->text(1)))
	{
		DrBase	*opt = m_opts[item->text(1)];
		opt->set("text", item->text(0));
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::recreateGroup(Q3ListViewItem *item, DrGroup *grp)
{
	if (!item)
		return;

	Q3ListViewItem	*child = item->firstChild();
	while (child)
	{
		DrBase	*opt = (m_opts.contains(child->text(1)) ? m_opts[child->text(1)] : 0);
		if (opt)
		{
			if (opt->type() == DrBase::Group)
			{
				DrGroup	*childGroup = static_cast<DrGroup*>(opt);
				recreateGroup(child, childGroup);
				grp->addGroup(childGroup);
			}
			else
			{
				opt->setName("_kde-"+m_xmlcmd->name()+"-"+opt->name());
				grp->addOption(opt);
			}
			m_opts.remove(child->text(1));
		}
		child = child->nextSibling();
	}
}

bool KXmlCommandAdvancedDlg::editCommand(KXmlCommand *xmlcmd, QWidget *parent)
{
	if (!xmlcmd)
		return false;

	KDialogBase	dlg(parent, 0, true, i18n("Command Edit for %1").arg(xmlcmd->name()), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, false);
	KXmlCommandAdvancedDlg	*xmldlg = new KXmlCommandAdvancedDlg(&dlg);
	dlg.setMainWidget(xmldlg);
	//dlg.enableButton(KDialogBase::Ok, false);
	xmldlg->setCommand(xmlcmd);
	if (dlg.exec())
	{
		xmlcmd->setCommand(xmldlg->m_command->text());
		xmlcmd->setIo(xmldlg->m_inputfile->text(), true, false);
		xmlcmd->setIo(xmldlg->m_inputpipe->text(), true, true);
		xmlcmd->setIo(xmldlg->m_outputfile->text(), false, false);
		xmlcmd->setIo(xmldlg->m_outputpipe->text(), false, true);
		xmlcmd->setComment( xmldlg->m_comment->text().replace( QRegExp( "\n" ), " " ) );

		// need to recreate the driver tree structure
		DrMain	*driver = (xmldlg->m_opts.contains("__root__") ? static_cast<DrMain*>(xmldlg->m_opts["__root__"]) : 0);
		if (!driver && xmldlg->m_opts.count() > 0)
		{
			kdDebug() << "KXmlCommandAdvancedDlg: driver structure not found, creating one" << endl;
			driver = new DrMain;
			driver->setName(xmlcmd->name());
		}
		xmldlg->recreateGroup(xmldlg->m_view->firstChild(), driver);
		xmldlg->m_opts.remove("__root__");
		xmlcmd->setDriver(driver);

		// remaining options will be removed in destructor

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------

KXmlCommandDlg::KXmlCommandDlg(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, QString::null, Ok|Cancel|Details, Ok, true)
{
	setButtonText(Details, i18n("&Mime Type Settings"));
	m_cmd = 0;

	QWidget	*dummy = new QWidget( this );
        dummy->setObjectName( "TopDetail");
	QWidget	*topmain = new QWidget(this );
        topmain->setObjectName( "TopMain");

	Q3GroupBox	*m_gb1 = new Q3GroupBox(0, Qt::Horizontal, i18n("Supported &Input Formats"), dummy);
	Q3GroupBox	*m_gb2 = new Q3GroupBox(0, Qt::Horizontal, i18n("Requirements"), topmain);

	m_description = new QLineEdit(topmain);
	m_idname = new QLabel(topmain);
	m_requirements = new KListView(m_gb2);
	m_requirements->addColumn("");
	m_requirements->header()->hide();
	m_addreq = new QToolButton(m_gb2);
	m_addreq->setIcon(SmallIconSet("filenew"));
	m_removereq = new QToolButton(m_gb2);
	m_removereq->setIcon(SmallIconSet("editdelete"));
	QPushButton	*m_edit = new KPushButton(KGuiItem(i18n("&Edit Command..."), "edit"), topmain);
	m_mimetype = new QComboBox(dummy);
	m_availablemime = new KListBox(m_gb1);
	m_selectedmime = new KListBox(m_gb1);
	m_addmime = new QToolButton(m_gb1);
	m_addmime->setIcon(QApplication::reverseLayout()? SmallIconSet("forward") : SmallIconSet("back"));
	m_removemime = new QToolButton(m_gb1);
	m_removemime->setIcon(QApplication::reverseLayout()? SmallIconSet("back" ) : SmallIconSet("forward"));
	m_gb2->setMinimumWidth(380);
	m_gb1->setMinimumHeight(180);
	m_requirements->setMaximumHeight(80);
	m_removereq->setEnabled(false);
	m_addmime->setEnabled(false);
	m_removemime->setEnabled(false);

	QLabel	*m_desclab = new QLabel(i18n("&Description:"), topmain);
	m_desclab->setBuddy(m_description);
	QLabel	*m_mimetypelab = new QLabel(i18n("Output &format:"), dummy);
	m_mimetypelab->setBuddy(m_mimetype);
	QLabel	*m_idnamelab = new QLabel(i18n("ID name:"), topmain);

	QFont	f(m_idname->font());
	f.setBold(true);
	m_idname->setFont(f);

	KSeparator	*sep1 = new KSeparator(Q3Frame::HLine, dummy);

	QVBoxLayout	*l0 = new QVBoxLayout(topmain, 0, 10);
	QGridLayout	*l5 = new QGridLayout(0, 2, 2, 0, 5);
	l0->addLayout(l5);
	l5->addWidget(m_idnamelab, 0, 0);
	l5->addWidget(m_idname, 0, 1);
	l5->addWidget(m_desclab, 1, 0);
	l5->addWidget(m_description, 1, 1);
	l0->addWidget(m_gb2);
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 0);
	l0->addLayout(l3);
	l3->addWidget(m_edit);
	l3->addStretch(1);

	QVBoxLayout	*l7 = new QVBoxLayout(dummy, 0, 10);
	QHBoxLayout	*l6 = new QHBoxLayout(0, 0, 5);
	l7->addWidget(sep1);
	l7->addLayout(l6);
	l6->addWidget(m_mimetypelab, 0);
	l6->addWidget(m_mimetype, 1);
	l7->addWidget(m_gb1);
	QGridLayout	*l2 = new QGridLayout(m_gb1->layout(), 4, 3, 10);
	l2->addMultiCellWidget(m_availablemime, 0, 3, 2, 2);
	l2->addMultiCellWidget(m_selectedmime, 0, 3, 0, 0);
	l2->addWidget(m_addmime, 1, 1);
	l2->addWidget(m_removemime, 2, 1);
	l2->setRowStretch(0, 1);
	l2->setRowStretch(3, 1);
	QHBoxLayout	*l4 = new QHBoxLayout(m_gb2->layout(), 10);
	l4->addWidget(m_requirements);
	QVBoxLayout	*l8 = new QVBoxLayout(0, 0, 0);
	l4->addLayout(l8);
	l8->addWidget(m_addreq);
	l8->addWidget(m_removereq);
	l8->addStretch(1);

	connect(m_addmime, SIGNAL(clicked()), SLOT(slotAddMime()));
	connect(m_removemime, SIGNAL(clicked()), SLOT(slotRemoveMime()));
	connect(m_edit, SIGNAL(clicked()), SLOT(slotEditCommand()));
	connect(m_requirements, SIGNAL(selectionChanged(Q3ListViewItem*)), SLOT(slotReqSelected(Q3ListViewItem*)));
	connect(m_availablemime, SIGNAL(selectionChanged(Q3ListBoxItem*)), SLOT(slotAvailableSelected(Q3ListBoxItem*)));
	connect(m_selectedmime, SIGNAL(selectionChanged(Q3ListBoxItem*)), SLOT(slotSelectedSelected(Q3ListBoxItem*)));
	connect(m_addreq, SIGNAL(clicked()), SLOT(slotAddReq()));
	connect(m_removereq, SIGNAL(clicked()), SLOT(slotRemoveReq()));

	KMimeType::List	list = KMimeType::allMimeTypes();
	for (Q3ValueList<KMimeType::Ptr>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		QString	mimetype = (*it)->name();
		m_mimelist << mimetype;
	}

	m_mimelist.sort();
	m_mimetype->insertStringList(m_mimelist);
	m_availablemime->insertStringList(m_mimelist);

	setMainWidget(topmain);
	setDetailsWidget(dummy);
}

void KXmlCommandDlg::setCommand(KXmlCommand *xmlCmd)
{
	setCaption(i18n("Command Edit for %1").arg(xmlCmd->name()));

	m_cmd = xmlCmd;
	m_description->setText(i18n(xmlCmd->description().utf8()));
	m_idname->setText(xmlCmd->name());

	m_requirements->clear();
	QStringList	list = xmlCmd->requirements();
	Q3ListViewItem	*item(0);
	for (QStringList::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		item = new Q3ListViewItem(m_requirements, item, *it);
		item->setRenameEnabled(0, true);
	}

	int	index = m_mimelist.findIndex(xmlCmd->mimeType());
	if (index != -1)
		m_mimetype->setCurrentItem(index);
	else
		m_mimetype->setCurrentItem(0);

	list = xmlCmd->inputMimeTypes();
	m_selectedmime->clear();
	m_availablemime->clear();
	m_availablemime->insertStringList(m_mimelist);
	for (QStringList::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		m_selectedmime->insertItem(*it);
		delete m_availablemime->findItem(*it, Q3ListView::ExactMatch);
	}
}

void KXmlCommandDlg::slotOk()
{
	if (m_cmd)
	{
		m_cmd->setMimeType((m_mimetype->currentText() == "all/all" ? QString::null : m_mimetype->currentText()));
		m_cmd->setDescription(m_description->text());
		QStringList	l;
		Q3ListViewItem	*item = m_requirements->firstChild();
		while (item)
		{
			l << item->text(0);
			item = item->nextSibling();
		}
		m_cmd->setRequirements(l);
		l.clear();
		for (uint i=0; i<m_selectedmime->count(); i++)
			l << m_selectedmime->text(i);
		m_cmd->setInputMimeTypes(l);
	}
	KDialogBase::slotOk();
}

bool KXmlCommandDlg::editCommand(KXmlCommand *xmlCmd, QWidget *parent)
{
	if (!xmlCmd)
		return false;

	KXmlCommandDlg	xmldlg(parent, 0);
	xmldlg.setCommand(xmlCmd);

	return (xmldlg.exec() == Accepted);
}

void KXmlCommandDlg::slotAddMime()
{
	int	index = m_availablemime->currentItem();
	if (index != -1)
	{
		m_selectedmime->insertItem(m_availablemime->currentText());
		m_availablemime->removeItem(index);
		m_selectedmime->sort();
	}
}

void KXmlCommandDlg::slotRemoveMime()
{
	int	index = m_selectedmime->currentItem();
	if (index != -1)
	{
		m_availablemime->insertItem(m_selectedmime->currentText());
		m_selectedmime->removeItem(index);
		m_availablemime->sort();
	}
}

void KXmlCommandDlg::slotEditCommand()
{
	KXmlCommandAdvancedDlg::editCommand(m_cmd, parentWidget());
}

void KXmlCommandDlg::slotAddReq()
{
	Q3ListViewItem	*item = new Q3ListViewItem(m_requirements, m_requirements->lastItem(), i18n("exec:/"));
	item->setRenameEnabled(0, true);
	m_requirements->ensureItemVisible(item);
	item->startRename(0);
}

void KXmlCommandDlg::slotRemoveReq()
{
	delete m_requirements->currentItem();
}

void KXmlCommandDlg::slotReqSelected(Q3ListViewItem *item)
{
	m_removereq->setEnabled(item);
}

void KXmlCommandDlg::slotAvailableSelected(Q3ListBoxItem *item)
{
	m_addmime->setEnabled(item);
}

void KXmlCommandDlg::slotSelectedSelected(Q3ListBoxItem *item)
{
	m_removemime->setEnabled(item);
}

#include "kxmlcommanddlg.moc"
