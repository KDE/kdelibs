#include "kprintsizecolorpage.h"
#include "kprinter.h"

#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <klocale.h>
#include <kiconloader.h>

typedef struct {
	int	index;
	int	ID;
	QString	name;
} format_entry;

static format_entry formats[] = {
	{0, KPrinter::A0, "A0"},
	{1, KPrinter::A1, "A1"},
	{2, KPrinter::A2, "A2"},
	{3, KPrinter::A3, "A3"},
	{4, KPrinter::A4, "A4"},
	{5, KPrinter::A5, "A5"},
	{6, KPrinter::A6, "A6"},
	{7, KPrinter::A7, "A7"},
	{8, KPrinter::A8, "A8"},
	{9, KPrinter::A9, "A9"},
	{10, KPrinter::B0, "B0"},
	{11, KPrinter::B1, "B1"},
	{12, KPrinter::B10, "B10"},
	{13, KPrinter::B2, "B2"},
	{14, KPrinter::B3, "B3"},
	{15, KPrinter::B4, "B4"},
	{16, KPrinter::B5, "B5"},
	{17, KPrinter::B6, "B6"},
	{18, KPrinter::B7, "B7"},
	{19, KPrinter::B8, "B8"},
	{20, KPrinter::B9, "B9"},
	{21, KPrinter::C5E, "Envelope C5"},
	{22, KPrinter::DLE, "Envelope DL"},
	{23, KPrinter::Comm10E, "Envelope #10 (US)"},
	{24, KPrinter::Executive, "Executive"},
	{25, KPrinter::Folio, "Folio"},
	{26, KPrinter::Ledger, "Ledger"},
	{27, KPrinter::Legal, "Legal"},
	{28, KPrinter::Letter, "Letter"},
	{29, KPrinter::Tabloid, "Tabloid"},
	{-1, -1, ""}
};

KPrinter::PageSize findSize(int index)
{
	return (KPrinter::PageSize)(formats[index].ID);
}

int findIndex(KPrinter::PageSize ps)
{
	int	i(0);
	while (formats[i].index != -1)
		if (formats[i].ID == ps)
			break;
		else
			i++;
	return formats[i].index;
}

KPrintSizeColorPage::KPrintSizeColorPage(QWidget *parent, const char *name)
: KPrintSizeColorPageBase(parent,name)
{
	initialize();
}

KPrintSizeColorPage::~KPrintSizeColorPage()
{
}

QString KPrintSizeColorPage::pageTitle()
{
	return i18n("Page / Color");
}

void KPrintSizeColorPage::getOptions(OptionSet& opts, bool)
{
	opts["qt-colormode"] = (grayscale_->isChecked() ? "GrayScale" : "Color");
	opts["orientation-requested"] = (landscape_->isChecked() ? "4" : "3");
	opts["qt-pagesize"] = QString::number((int)(formats[size_->currentItem()].ID));
}

void KPrintSizeColorPage::setOptions(const OptionSet& opts)
{
	QString	value;

	value = opts["qt-pagesize"];
	int	p;
	if (!value.isEmpty())
	{
		p = findIndex((KPrinter::PageSize)(value.toInt()));
		if (p != -1) size_->setCurrentItem(p);
	}

	value = opts["qt-colormode"];
	if (!value.isEmpty())
	{
		p = (value == "GrayScale" ? 1 : 0);
		colorbox_->setButton(p);
		colorChanged(p);
	}

	value = opts["orientation-requested"];
	if (!value.isEmpty())
	{
		p = (value == "4" ? 1 : 0);
		orientbox_->setButton(p);
		orientationChanged(p);
	}
}

void KPrintSizeColorPage::initialize()
{
	connect(orientbox_,SIGNAL(clicked(int)),SLOT(orientationChanged(int)));
	connect(colorbox_,SIGNAL(clicked(int)),SLOT(colorChanged(int)));

	color_->setChecked(true);
	portrait_->setChecked(true);

	orientationChanged(0);
	colorChanged(0);

	int	i(0);
	QPixmap	pix = UserIcon("kdeprint_pagesize");
	while (formats[i].index != -1)
		size_->insertItem(pix,formats[i++].name);
	size_->setCurrentItem(findIndex(KPrinter::A4));
}

void KPrintSizeColorPage::orientationChanged(int o)
{
	if (o == 0) orientpix_->setPixmap(UserIcon("kdeprint_portrait"));
	else orientpix_->setPixmap(UserIcon("kdeprint_landscape"));
}

void KPrintSizeColorPage::colorChanged(int o)
{
	if (o == 0) colorpix_->setPixmap(UserIcon("kdeprint_color"));
	else colorpix_->setPixmap(UserIcon("kdeprint_grayscale"));
}
