#include "kmpropmembers.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qtextview.h>
#include <qlayout.h>
#include <klocale.h>

KMPropMembers::KMPropMembers(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_members = new QTextView(this);
	m_members->setPaper(colorGroup().background());
	m_members->setFrameStyle(QFrame::NoFrame);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 0);
	main_->addWidget(m_members);

	m_pixmap = "kdeprint_printer_class";
	m_title = i18n("Members");
	m_header = i18n("Class members");
}

KMPropMembers::~KMPropMembers()
{
}

void KMPropMembers::setPrinter(KMPrinter *p)
{
	if (p && p->isClass(false) && p->isLocal())
	{
		QStringList	l = p->members();
		QString		txt("<ul>");
		for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
			txt.append("<li>" + (*it) + "</li>");
		txt.append("</ul>");
		m_members->setText(txt);
		emit enable(true);
	}
	else
	{
		emit enable(false);
		m_members->setText("");
	}
}

void KMPropMembers::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Class,KMWizard::Class,true);
}
