#include "kppdwidget.h"
#include "kppdview.h"
#include "kppdoptionview.h"

#include <qlayout.h>

KPPDWidget::KPPDWidget(QWidget *parent, const char *name)
	: QWidget(parent,name)
{
	ppdview_ = new KPPDView(NULL,this);
	ppdoptview_ = new KPPDOptionView(this);

	QObject::connect(ppdview_,SIGNAL(selectionChanged(QListViewItem*)),ppdoptview_,SLOT(setOption(QListViewItem*)));
	QObject::connect(ppdoptview_,SIGNAL(optionChanged()),ppdview_,SLOT(checkConstraints()));

	QVBoxLayout	*mainLayout_ = new QVBoxLayout(this, 10, 10);
	mainLayout_->addWidget(ppdview_);
	mainLayout_->addWidget(ppdoptview_);
}

KPPDWidget::~KPPDWidget()
{
}

void KPPDWidget::initialize(global_ppd_file_t *ppd)
{
	ppdview_->initialize(ppd);
}

void KPPDWidget::markChoices(global_ppd_file_t *ppd)
{
	ppdview_->markChoices(ppd);
}

void KPPDWidget::updateChoices(global_ppd_file_t *ppd)
{
	ppdview_->updateChoices(ppd);
	ppdoptview_->setOption(ppdview_->currentItem());
}

void KPPDWidget::getOptions(OptionSet& opts, bool incldef)
{
	ppdview_->getOptions(opts,incldef);
}

void KPPDWidget::setOptions(const OptionSet& opts)
{
	ppdview_->setOptions(opts);
}

void KPPDWidget::setAllowNonChangeable(bool on)
{
	ppdoptview_->setAllowNonChangeable(on);
}

bool KPPDWidget::allowNonChangeable() const
{
	return ppdoptview_->allowNonChangeable();
}

bool KPPDWidget::hasConflict()
{
        return ppdview_->hasConflict();
}
