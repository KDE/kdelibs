#include "kcupshpgl2page.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <klocale.h>

KCupsHPGL2Page::KCupsHPGL2Page(global_ppd_file_t*, IppRequest*, QWidget *parent, const char *name)
: KCupsHPGL2PageBase(parent,name)
{
	initialize();
}

KCupsHPGL2Page::~KCupsHPGL2Page()
{
}

QString KCupsHPGL2Page::pageTitle()
{
	return i18n("HP-GL/2");
}

void KCupsHPGL2Page::initialize()
{
	penwidth_->setRange(0,10000);
	penwidth_->setSteps(25,25);
	penwidth_->setValue(1000);

	unitlabel_->setText("[µm]");
}

void KCupsHPGL2Page::setOptions(const OptionSet& opts)
{
	black_->setChecked(opts.contains("blackpot"));
	fit_->setChecked(opts.contains("fitplot"));
	if (!opts["penwidth"].isEmpty())
		penwidth_->setValue(opts["penwidth"].toInt());
}

void KCupsHPGL2Page::getOptions(OptionSet& opts, bool incldef)
{
	if (black_->isChecked()) opts["blackpot"] = "";
	else opts.remove("blackpot");
	if (fit_->isChecked()) opts["fitplot"] = "";
	else opts.remove("fitplot");
	if (incldef || penwidth_->value() != 1000)
		opts["penwidth"] = penwidth_->text();
}
