#include "kmwclass.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <klistbox.h>
#include <klocale.h>
#include <kiconloader.h>

KMWClass::KMWClass(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Class;
	m_title = i18n("Class composition");
	m_nextpage = KMWizard::Name;

	m_list1 = new KListBox(this);
	m_list1->setSelectionMode(QListBox::Extended);
	m_list2 = new KListBox(this);
	m_list2->setSelectionMode(QListBox::Extended);

	QToolButton	*add = new QToolButton(this);
	QToolButton	*remove = new QToolButton(this);
	add->setIconSet(BarIcon("forward"));
	remove->setIconSet(BarIcon("back"));
	connect(add,SIGNAL(clicked()),SLOT(slotAdd()));
	connect(remove,SIGNAL(clicked()),SLOT(slotRemove()));

	QLabel	*l1 = new QLabel(i18n("Available printers"), this);
	QLabel	*l2 = new QLabel(i18n("Class printers"), this);

        QHBoxLayout	*lay1 = new QHBoxLayout(this, 0, 15);
        QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 20);
        QVBoxLayout	*lay3 = new QVBoxLayout(0, 0, 0), *lay4 = new QVBoxLayout(0, 0, 0);
        lay1->addLayout(lay3, 1);
        lay1->addLayout(lay2, 0);
	lay1->addLayout(lay4, 1);
        lay3->addWidget(l1, 0);
        lay3->addWidget(m_list1, 1);
        lay2->addStretch(1);
        lay2->addWidget(add, 0);
        lay2->addWidget(remove, 0);
        lay2->addStretch(1);
        lay4->addWidget(l2, 0);
        lay4->addWidget(m_list2, 1);
}

KMWClass::~KMWClass()
{
}

bool KMWClass::isValid(QString& msg)
{
	if (m_list2->count() == 0)
	{
		msg = i18n("You must select at least one printer !");
		return false;
	}
	return true;
}

void KMWClass::initPrinter(KMPrinter *p)
{
	QStringList	members = p->members();

	// first load available printers
	QList<KMPrinter>	*list = KMFactory::self()->manager()->printerList(false);
	m_list1->clear();
	if (list)
	{
		QListIterator<KMPrinter>	it(*list);
		for (;it.current();++it)
			if (it.current()->instanceName().isEmpty() && !it.current()->isClass(true) && !members.contains(it.current()->name()))
				m_list1->insertItem(it.current()->name());
		m_list1->sort();
	}

	// set class printers
	m_list2->clear();
	m_list2->insertStringList(members);
	m_list2->sort();
}

void KMWClass::updatePrinter(KMPrinter *p)
{
	QStringList	members;
	for (uint i=0; i<m_list2->count(); i++)
		members.append(m_list2->item(i)->text());
	p->setMembers(members);
}

void KMWClass::slotAdd()
{
	for (uint i=0;i<m_list1->count();i++)
		if (m_list1->isSelected(i))
		{
			m_list2->insertItem(m_list1->text(i));
			m_list1->removeItem(i--);
		}
	m_list2->sort();
}

void KMWClass::slotRemove()
{
	for (uint i=0;i<m_list2->count();i++)
		if (m_list2->isSelected(i))
		{
			m_list1->insertItem(m_list2->text(i));
			m_list2->removeItem(i--);
		}
	m_list1->sort();
}
#include "kmwclass.moc"
