#include "kmpropcontainer.h"
#include "kmpropwidget.h"

#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>

KMPropContainer::KMPropContainer(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	QFrame	*line = new QFrame(this);
	line->setFrameStyle(QFrame::HLine|QFrame::Sunken);
	line->setLineWidth(1);
	line->setFixedHeight(5);

	m_button = new QPushButton(i18n("Change..."), this);
	m_widget = 0;

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 0);
	main_->addWidget(line);
	main_->addLayout(btn_);
	btn_->addStretch(1);
	btn_->addWidget(m_button);
}

KMPropContainer::~KMPropContainer()
{
}

void KMPropContainer::setWidget(KMPropWidget *w)
{
	if (!m_widget)
	{
		m_widget = w;
		m_widget->reparent(this,QPoint(0,0));
		connect(m_button,SIGNAL(clicked()),m_widget,SLOT(slotChange()));
		connect(m_widget,SIGNAL(enable(bool)),SIGNAL(enable(bool)));
		connect(m_widget,SIGNAL(enableChange(bool)),m_button,SLOT(setEnabled(bool)));
		QVBoxLayout	*lay = dynamic_cast<QVBoxLayout*>(layout());
		if (lay) lay->insertWidget(0,m_widget);
	}
}

void KMPropContainer::setPrinter(KMPrinter *p)
{
	if (m_widget)
		m_widget->setPrinterBase(p);
}
#include "kmpropcontainer.moc"
