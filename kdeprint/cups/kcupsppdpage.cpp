#include "kcupsppdpage.h"
#include "kppdwidget.h"
#include "ppd-util.h"

#include <qlayout.h>
#include <klocale.h>

KCupsPPDPage::KCupsPPDPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent, const char *name)
: KPrintDialogPage(parent,name), ppd_(ppd), request_(req)
{
	initialize();
}

KCupsPPDPage::~KCupsPPDPage()
{
}

void KCupsPPDPage::initialize()
{
	widget_ = new KPPDWidget(this);
	widget_->initialize(ppd_);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 0);
	main_->addWidget(widget_);
}

bool KCupsPPDPage::isValid()
{
	return !(widget_->hasConflict());
}

void KCupsPPDPage::setOptions(const OptionSet& opts)
{
	widget_->setOptions(opts);
}

void KCupsPPDPage::getOptions(OptionSet& opts, bool incldef)
{
	widget_->getOptions(opts,incldef);
}

QString KCupsPPDPage::pageTitle()
{
	return i18n("Advanced");
}
