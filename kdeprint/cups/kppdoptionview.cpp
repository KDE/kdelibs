#include "kppdoptionview.h"
#include "kppdoptionitem.h"
#include "kppdnumericaloptionitem.h"
#include "numericwidget.h"

#include <qlistbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <klocale.h>

#include <cups/ppd.h>

#define	OFF_ID	0
#define	ON_ID	1

KPPDOptionView::KPPDOptionView(QWidget *parent, const char *name)
	: QGroupBox(parent,name)
{
	ppdopt_ = 0;
	block_ = false;

	setTitle( i18n("No option selected") );
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum));

	stack_ = new QWidgetStack(this);
	optionpickone_ = new QListBox(stack_);
	optionboolean_ = new QButtonGroup(stack_);
	optionnumeric_ = new NumericWidget(stack_);
	optionempty_ = new QWidget(stack_);
	stack_->addWidget(optionpickone_, 1);
	stack_->addWidget(optionboolean_, 2);
	stack_->addWidget(optionnumeric_, 3);
	stack_->addWidget(optionempty_, 0);
	stack_->raiseWidget(optionempty_);

	// Radio buttons creation
	QVBoxLayout	*boolLayout_ = new QVBoxLayout(optionboolean_, 5, 0);
	boolLayout_->addWidget(new QRadioButton(i18n("Off"), optionboolean_), 1);
	boolLayout_->addWidget(new QRadioButton(i18n("On"), optionboolean_), 1);
	optionboolean_->setTitle(QString::fromLatin1(""));
	optionboolean_->setFrameStyle(QFrame::NoFrame);

	allowNonChangeable_ = false;

	connect(optionboolean_,SIGNAL(clicked(int)),SLOT(selectionChanged(int)));
	connect(optionpickone_,SIGNAL(highlighted(int)),SLOT(selectionChanged(int)));
	connect(optionnumeric_,SIGNAL(valueChanged(double)),SLOT(valueChanged(double)));

	QVBoxLayout	*mainLayout_ = new QVBoxLayout(this, 11, 0);
	mainLayout_->addSpacing(10);
	mainLayout_->addWidget(stack_);
}

KPPDOptionView::~KPPDOptionView()
{
}

QSize KPPDOptionView::sizeHint() const
{
	return QSize(200,120);
}

void KPPDOptionView::setOption(QListViewItem *item)
{
	// check if option selected
	if (!item || ((KPPDBaseItem*)item)->isGroup())
	{
		ppdopt_ = 0;
		setTitle( i18n("No option selected") );
		stack_->raiseWidget(optionempty_);
		return;
	}

	ppdopt_ = (KPPDBaseItem*)item;

	// we are constructing from an option -> block updating the option
	// in PPD view to avoid any side-effects (no need to update).
	block_ = true;

	if (ppdopt_->isNumeric())
	{
		// option selected
		KPPDNumericalOptionItem	*opt_ = (KPPDNumericalOptionItem*)ppdopt_;
		setTitle(opt_->option_->text);

		stack_->raiseWidget(optionnumeric_);
		if (opt_->option_->type != 0) optionnumeric_->setFactor(1000);
		optionnumeric_->setLimits(opt_->option_->min, opt_->option_->max);
		optionnumeric_->setValue(opt_->option_->value);
	}
	else
	{
		// option selected
		KPPDOptionItem	*opt_ = (KPPDOptionItem*)ppdopt_;
		setTitle(opt_->option_->text);

		// raising widget
		switch(opt_->option_->ui)
		{
			case PPD_UI_BOOLEAN:
				if (opt_->option_->num_choices == 2)	// we must have 2 choices for a boolean option
				{
					optionboolean_->find(OFF_ID)->setText(opt_->option_->choices[0].text);
					optionboolean_->find(ON_ID)->setText(opt_->option_->choices[1].text);
					if (opt_->currentchoice_ == opt_->option_->choices)
						optionboolean_->setButton(OFF_ID);
					else
						optionboolean_->setButton(ON_ID);
				}
				stack_->raiseWidget(optionboolean_);
		   	break;
			case PPD_UI_PICKONE:
				optionpickone_->clear();
			   	for (int i=0;i<opt_->option_->num_choices;i++)
			   	{
			   		ppd_choice_t	*ch = opt_->option_->choices+i;
					optionpickone_->insertItem(ch->text);
			   		if (opt_->currentchoice_ == ch) optionpickone_->setCurrentItem(i);
			   	}
			   	optionpickone_->centerCurrentItem();
				stack_->raiseWidget(optionpickone_);
				break;
			case PPD_UI_PICKMANY:
				break;
		}
		stack_->setEnabled((opt_->changeable_ || allowNonChangeable_));
	}

	block_ = false;
}

void KPPDOptionView::selectionChanged(int index)
{
	if (!block_)
	{
		if (ppdopt_ && ppdopt_->isOption()) ((KPPDOptionItem*)ppdopt_)->setCurrentChoice(index);
		emit optionChanged();
	}
}

void KPPDOptionView::valueChanged(double value)
{
	if (!block_)
		if (ppdopt_ && ppdopt_->isNumeric())
			((KPPDNumericalOptionItem*)ppdopt_)->setValue(value);
	// do not check constraints for numerical options changes.
}
#include "kppdoptionview.moc"
