#include "kprintcopiespage.h"
#include "kprinter.h"
#include "kdialogimpl.h"

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <klocale.h>
#include <kiconloader.h>

KPrintCopiesPage::KPrintCopiesPage(QWidget *parent, const char *name)
: KPrintCopiesPageBase(parent,name)
{
	connect(collate_,SIGNAL(clicked()),SLOT(collateClicked()));
	connect(reverse_,SIGNAL(clicked()),SLOT(collateClicked()));
	connect(rangeedit_,SIGNAL(textChanged(const QString&)),SLOT(rangeEntered()));

	int	fl = KPrinterFactory::self()->printer()->copyFlags();

	current_->setEnabled((fl & KDialogImpl::Current));
	range_->setEnabled((fl & KDialogImpl::Range));
	rangeedit_->setEnabled((fl & KDialogImpl::Range));
	rangeexpl_->setEnabled((fl & KDialogImpl::Range));
	collate_->setEnabled((fl & KDialogImpl::Collate));
	reverse_->setEnabled((fl & KDialogImpl::Reverse));
	pagecombo_->setEnabled((fl & KDialogImpl::PageSet));

	all_->setChecked(true);
	if (collate_->isEnabled()) collate_->setChecked(true);
	collateClicked();
}

KPrintCopiesPage::~KPrintCopiesPage()
{
}

void KPrintCopiesPage::setOptions(const OptionSet& options)
{
	QString	value;
	// copies
	value = options["qt-copies"];
	if (!value.isEmpty()) copies_->setValue(value.toInt());
	// output order
	value = options["qt-pageorder"];
	if (reverse_->isEnabled()) reverse_->setChecked(value == "Reverse");
	// collate
	value = options["qt-collate"];
	if (collate_->isEnabled()) collate_->setChecked(value.isEmpty() || value == "Collate");
	// update pixmap
	collateClicked();
	// page ranges
	value = options["qt-range"];
	if (!value.isEmpty() && range_->isEnabled())
	{
		rangeedit_->setText(value);
		range_->setChecked(true);
	}
	else if (options["qt-current"] == "1")
		current_->setChecked(true);
	else
		all_->setChecked(true);
	// page set
	value = options["qt-pageset"];
	if (!value.isEmpty() && pagecombo_->isEnabled())
		pagecombo_->setCurrentItem(value.toInt());
	else
		pagecombo_->setCurrentItem(0);
}

void KPrintCopiesPage::getOptions(OptionSet& options, bool)
{
	// copies
	options["qt-copies"] = copies_->text();
	// output order
	options["qt-pageorder"] = (reverse_->isChecked() ? "Reverse" : "Forward");
	// collate
	options["qt-collate"] = (collate_->isChecked() ? "Collate" : "Uncollate");
	// ranges
	options["qt-current"] = (current_->isChecked() ? "1" : "0");
	options["qt-range"] = (range_->isChecked() ? rangeedit_->text() : QString::fromLatin1(""));
	// page set
	options["qt-pageset"] = QString::number(pagecombo_->currentItem());
}

QString KPrintCopiesPage::pageTitle()
{
	return i18n("Copies");
}

void KPrintCopiesPage::collateClicked()
{
	QString pix(collate_->isChecked() ? (reverse_->isChecked() ? "collate_rev" : "collate") : (reverse_->isChecked() ? "uncollate_rev" : "uncollate"));
	copiespix_->setPixmap(UserIcon("kdeprint_"+pix));
}

void KPrintCopiesPage::rangeEntered()
{
	if (!range_->isChecked()) range_->setChecked(true);
}
#include "kprintcopiespage.moc"
