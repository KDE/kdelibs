#include "kcupstextpage.h"
#include "qclineedit.h"
#include "marginpreview.h"
#include "columnpreview.h"

#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapp.h>

static ppd_size_t* findDefaultFromStr(const char *str);
static ppd_size_t* findDefault(const char *mediastr);
static int uncalcDim(float dim, int unit);
static void convertValue(QLineEdit *e, int old_units, int new_units);
static void calcDimStr(int pixels, int unit, QString& str);
static float calcDim(int pixels, int unit);

#define	ON_ID	0
#define	OFF_ID	1

#define	UNIT_CM		0
#define	UNIT_IN		1
#define	UNIT_PIXEL	2

#define	DEFAULT_PAGESIZE	5
static ppd_size_t	default_pagesize[] =
{
	{0,"Letter",612,792,18,36,594,756},
	{0,"Legal",612,1008,18,36,594,972},
	{0,"A4",595,842,18,36,577,806},
	{0,"COM10",297,684,18,36,279,648},
	{0,"DL",312,624,18,36,294,588}
};

//*************************************************************************************************

KCupsTextPage::KCupsTextPage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent, const char *name)
: KCupsTextPageBase(parent,name), ppd_(ppd), request_(req)
{
	orientation_ = 3;
	pagesize_ = 0;

	initialize();
}

KCupsTextPage::~KCupsTextPage()
{
}

void KCupsTextPage::initialize()
{
	connect(bottom_,SIGNAL(returnPressed()),SLOT(updateMargins()));
	connect(top_,SIGNAL(returnPressed()),SLOT(updateMargins()));
	connect(right_,SIGNAL(returnPressed()),SLOT(updateMargins()));
	connect(left_,SIGNAL(returnPressed()),SLOT(updateMargins()));
	connect(prettybox_,SIGNAL(clicked(int)),SLOT(prettyChanged(int)));
	connect(units_,SIGNAL(activated(int)),SLOT(unitsChanged(int)));
	connect(margincustom_,SIGNAL(clicked()),SLOT(customMarginsClicked()));
	connect(column_,SIGNAL(valueChanged(int)),columnpreview_,SLOT(setColumns(int)));

	prettyoff_->setChecked(true);
	prettyChanged(OFF_ID);
	customMarginsClicked();
	currentunits_ = UNIT_CM;
}

void KCupsTextPage::setOptions(const OptionSet& opts)
{
	// orientation
	if (!opts["orientation-requested"].isEmpty())
		orientation_ = opts["orientation-requested"].toInt();

	// pretty print option
	if (opts.contains("prettyprint"))
	{
		prettyon_->setChecked(true);
		prettyChanged(ON_ID);
	}

	// cpi and lpi
	if (!opts["cpi"].isEmpty())
		cpi_->setValue(opts["cpi"].toInt());
	if (!opts["lpi"].isEmpty())
		lpi_->setValue(opts["lpi"].toInt());

	// columns
	if (!opts["columns"].isEmpty())
		column_->setValue(opts["columns"].toInt());

	// margins
	QString	value;
	// first retrieve page size
	if (ppd_ && ppd_->ppd)
	{
		ppd_choice_t	*ch = ppdFindMarkedChoice(ppd_->ppd,"PageSize");
		QString	sz = (opts["PageSize"].isEmpty() ? (ch ? QString::fromLatin1(ch->choice) : QString::fromLatin1("A4")) : opts["PageSize"]);
		pagesize_ = ppdPageSize(ppd_->ppd,sz.latin1());
		// check if PPD file contains enough infos, otherwise use default page sizes
		if (!sz.isEmpty() && (!pagesize_ || pagesize_->width == 0 || pagesize_->length == 0))
			pagesize_ = findDefault(sz.latin1());
	}
	else
	{
		if (!opts["media"].isEmpty())
			pagesize_ = findDefault(opts["media"].latin1());
		else
			// default -> take first default (Letter)
			pagesize_ = default_pagesize;
	}
	// ... and initialize margin values
	if (!pagesize_) marginpreview_->hide();
	else
	{
		marginpreview_->show();
		updatePagesize();
	}
	updateDefaultMargins();
	// then set custom margins
	bool	marginset(false);
	if (!opts["page-top"].isEmpty())
	{
		marginset = true;
		calcDimStr(opts["page-top"].toInt(),currentunits_,value);
		top_->setText(value);
	}
	if (!opts["page-bottom"].isEmpty())
	{
		marginset = true;
		calcDimStr(opts["page-bottom"].toInt(),currentunits_,value);
		bottom_->setText(value);
	}
	if (!opts["page-left"].isEmpty())
	{
		marginset = true;
		calcDimStr(opts["page-left"].toInt(),currentunits_,value);
		left_->setText(value);
	}
	if (!opts["page-right"].isEmpty())
	{
		marginset = true;
		calcDimStr(opts["page-right"].toInt(),currentunits_,value);
		right_->setText(value);
	}
	if (!opts["margin-flag"].isEmpty())
		marginset = (opts["margin-flag"] == "on");
	if (marginset)
	{
		margincustom_->setChecked(true);
		customMarginsClicked();
		updateMargins();
	}
}

void KCupsTextPage::getOptions(OptionSet& opts, bool incldef)
{
	// pretty print
	if (prettyon_->isChecked())
		opts["prettyprint"] = "";
	else
		opts.remove("prettyprint");

	// cpi, lpi	and columns
	if (incldef || cpi_->value() != 10)
		opts["cpi"] = cpi_->text();
	if (incldef || lpi_->value() != 6)
		opts["lpi"] = lpi_->text();
	if (incldef || column_->value() != 1)
		opts["columns"] = column_->text();

	// margins
	if (margincustom_->isChecked())
	{
		QString	value;
		value.setNum(uncalcDim(QString(top_->text()).toFloat(),currentunits_));
		opts["page-top"] = value;
		value.setNum(uncalcDim(QString(bottom_->text()).toFloat(),currentunits_));
		opts["page-bottom"] = value;
		value.setNum(uncalcDim(QString(left_->text()).toFloat(),currentunits_));
		opts["page-left"] = value;
		value.setNum(uncalcDim(QString(right_->text()).toFloat(),currentunits_));
		opts["page-right"] = value;
	}
	if (incldef)
		opts["margin-flag"] = (margincustom_->isChecked() ? "on" : "off");
}

QString KCupsTextPage::pageTitle()
{
	return i18n("Text");
}

void KCupsTextPage::updateMargins()
{
	bool	ok;
	int	t(-1),b(-1),l(-1),r(-1);
	float	v;

	v = QString(top_->text()).toFloat(&ok);
	if (!ok || (t = uncalcDim(v,currentunits_)) < 0) goto end;
	v = QString(bottom_->text()).toFloat(&ok);
	if (!ok || (b = uncalcDim(v,currentunits_)) < 0) goto end;
	v = QString(left_->text()).toFloat(&ok);
	if (!ok || (l = uncalcDim(v,currentunits_)) < 0) goto end;
	v = QString(right_->text()).toFloat(&ok);
	if (!ok || (r = uncalcDim(v,currentunits_)) < 0) goto end;
	marginpreview_->setMargins(t,b,l,r);
	return;
end:
	kapp->beep();
	if (t < 0) top_->selectAll();
	else if (b < 0) bottom_->selectAll();
	else if (l < 0) left_->selectAll();
	else if (r < 0) right_->selectAll();
	return;
}

void KCupsTextPage::customMarginsClicked()
{
	bool	value(margincustom_->isChecked());
	top_->setEnabled(value);
	bottom_->setEnabled(value);
	left_->setEnabled(value);
	right_->setEnabled(value);
	units_->setEnabled(value);
	if (!value) updateDefaultMargins();
}

void KCupsTextPage::prettyChanged(int ID)
{
	if (ID == ON_ID) prettypix_->setPixmap(UserIcon("kdeprint_pretty"));
	else prettypix_->setPixmap(UserIcon("kdeprint_nup1"));
}

void KCupsTextPage::unitsChanged(int u)
{
	if (currentunits_ != u)
	{
		convertValue(top_,currentunits_,u);
		convertValue(bottom_,currentunits_,u);
		convertValue(left_,currentunits_,u);
		convertValue(right_,currentunits_,u);
		currentunits_ = u;
	}
}

void KCupsTextPage::updatePagesize()
{
	if (!pagesize_) return;
	if (orientation_ == 4 || orientation_ == 5)
		marginpreview_->setPageSize(pagesize_->length,pagesize_->width);
	else
		marginpreview_->setPageSize(pagesize_->width,pagesize_->length);
}

void KCupsTextPage::updateDefaultMargins()
{
	if (!pagesize_) return;
	int	v[4] = {(int)pagesize_->left, (int)pagesize_->bottom, (int)(pagesize_->width-pagesize_->right), (int)(pagesize_->length-pagesize_->top)};
	int	i = orientation_-3;
	if (i > 1) i ^= 1;
	int	l(v[i%4]), b(v[(i+1)%4]), r(v[(i+2)%4]), t(v[(i+3)%4]);
	marginpreview_->setMargins(t,b,l,r);

	QString	value;
	calcDimStr(t,currentunits_,value);
	top_->setText(value);
	calcDimStr(b,currentunits_,value);
	bottom_->setText(value);
	calcDimStr(l,currentunits_,value);
	left_->setText(value);
	calcDimStr(r,currentunits_,value);
	right_->setText(value);
}

//*************************************************************************************************

static ppd_size_t* findDefaultFromStr(const char *str)
{
	for (int i=0;i<DEFAULT_PAGESIZE;i++)
		if (strcasecmp(str,default_pagesize[i].name) == 0) return default_pagesize+i;
	return NULL;
}

static ppd_size_t* findDefault(const char *mediastr)
{
	char	*s = new char[strlen(mediastr)+2], *c;
	ppd_size_t	*sz(0);
	strcpy(s,mediastr);
	strcat(s,",");
	c = strtok(s,",");
	while (c && !sz) {
		sz = findDefaultFromStr(c);
		c = strtok(NULL,",");
	}
	delete [] s;
	return sz;
}

float calcDim(int pixels, int unit)
{
	switch (unit) {
		case 0: // cm
			return ((pixels / 72.0f) * 2.54f);
		case 1: // inches
			return (pixels / 72.0f);
		default:
			return (float)(pixels);
	}
}

void calcDimStr(int pixels, int unit, QString& str)
{
	float	value = calcDim(pixels,unit);
	if (unit == UNIT_PIXEL) str.setNum(pixels);
	else str.setNum(value,'f',3);
}

int uncalcDim(float dim, int unit)
{
	switch (unit) {
		case 0: // cm
			return (int)((dim / 2.54f) * 72.0f);
		case 1: // inches
			return (int)(dim * 72.0f);
		default:
			return (int)(dim);
	}
}

void convertValue(QLineEdit *e, int old_units, int new_units)
{
	QString	value;
	calcDimStr(uncalcDim(QString(e->text()).toFloat(),old_units),new_units,value);
	e->setText(value);
}
#include "kcupstextpage.moc"
