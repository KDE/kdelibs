#include "kpdqview.h"
#include "kpdqbaseitem.h"
#include "kpdqoptionitem.h"
#include "kpdqargitem.h"
#include "pdqclass.h"

#include <qheader.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kapp.h>
#include <qevent.h>

class PdqToolTip : public QToolTip
{
public:
	PdqToolTip(QListView *parent);
	virtual ~PdqToolTip();
protected:
	void maybeTip(const QPoint&);
private:
	QListView	*lv_;
};

PdqToolTip::PdqToolTip(QListView *parent)
: QToolTip(parent), lv_(parent)
{
}

PdqToolTip::~PdqToolTip()
{
}

void PdqToolTip::maybeTip(const QPoint& p)
{
	KPdqBaseItem	*item = (KPdqBaseItem*)(lv_->itemAt(p));
	if (item && !item->toolTip().isEmpty())
		tip(lv_->itemRect(item),item->toolTip());
}

class PdqWhatsThis : public QWhatsThis
{
public:
	PdqWhatsThis(QListView *lv);
	virtual ~PdqWhatsThis();
	QString text(const QPoint&);
private:
	QListView	*lv_;
};

PdqWhatsThis::PdqWhatsThis(QListView *lv)
: QWhatsThis(lv), lv_(lv)
{
}

PdqWhatsThis::~PdqWhatsThis()
{
}

QString PdqWhatsThis::text(const QPoint& p)
{
	KPdqBaseItem	*item = (KPdqBaseItem*)(lv_->itemAt(p));
	if (item && !item->toolTip().isEmpty())
		return item->toolTip();
	else
		return QString::null;
}

//***********************************************************************************************

KPdqView::KPdqView(QWidget *parent, const char *name)
: QListView(parent,name)
{
	tip_ = 0;
	whatsthis_ = new PdqWhatsThis(this);

	addColumn("Text");
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	header()->hide();
	setSorting(-1);

	root_ = 0;
	printer_ = 0;
	driver_ = 0;
	interface_ = 0;
}

KPdqView::~KPdqView()
{
	if (tip_) delete tip_;
	if (whatsthis_) delete whatsthis_;
}

void KPdqView::init(PDQPrinter *pr, PDQDriver *dr, PDQInterface *in)
{
	printer_ = pr;
	driver_ = dr;
	interface_ = in;

	if (!printer_) return;
	root_ = new KPdqBaseItem(this, i18n("Printer %1").arg(printer_->name()));
	root_->setPixmap(0,UserIcon("kdeprint_printer"));

	if (interface_)
	{
		KPdqBaseItem	*in_root = new KPdqBaseItem(root_,0,i18n("Interface options (%1)").arg(in->name()));
		in_root->setToolTip(interface_->help_);
		in_root->setPixmap(0,SmallIcon("connect_established"));
		if (interface_->arguments_.count() > 0)
		{
			KPdqBaseItem	*arg_root = new KPdqBaseItem(in_root,0,i18n("Arguments"));
			KPdqArgItem	*i(0);
			arg_root->setPixmap(0,SmallIcon("package"));
			QListIterator<PDQArgument>	it(interface_->arguments_);
			for (;it.current();++it)
			{
				i = new KPdqArgItem(it.current(),arg_root,i,"");
				i->setPixmap(0,UserIcon("kdeprint_opt_inputslot"));
			}
		}
		if (interface_->options_.count() > 0)
		{
			KPdqBaseItem	*opt_root = new KPdqBaseItem(in_root,0,i18n("Options"));
			KPdqOptionItem	*i(0);
			opt_root->setPixmap(0,SmallIcon("package"));
			QListIterator<PDQOption>	it(interface_->options_);
			for (;it.current();++it)
			{
				i = new KPdqOptionItem(it.current(),opt_root,i,"");
				i->setPixmap(0,UserIcon("kdeprint_opt_inputslot"));
			}
		}
	}

	if (driver_)
	{
		KPdqBaseItem	*dr_root = new KPdqBaseItem(root_,0,i18n("Driver options (%1)").arg(dr->name()));
		dr_root->setToolTip(driver_->help_);
		dr_root->setPixmap(0,SmallIcon("gear"));
		if (driver_->arguments_.count() > 0)
		{
			KPdqBaseItem	*arg_root = new KPdqBaseItem(dr_root,0,i18n("Arguments"));
			KPdqArgItem	*i(0);
			arg_root->setPixmap(0,SmallIcon("package"));
			QListIterator<PDQArgument>	it(driver_->arguments_);
			for (;it.current();++it)
			{
				i = new KPdqArgItem(it.current(),arg_root,i,"");
				i->setPixmap(0,UserIcon("kdeprint_opt_inputslot"));
			}
		}
		if (driver_->options_.count() > 0)
		{
			KPdqBaseItem	*opt_root = new KPdqBaseItem(dr_root,0,i18n("Options"));
			KPdqOptionItem	*i(0);
			opt_root->setPixmap(0,SmallIcon("package"));
			QListIterator<PDQOption>	it(driver_->options_);
			for (;it.current();++it)
			{
				i = new KPdqOptionItem(it.current(),opt_root,i,"");
				i->setPixmap(0,UserIcon("kdeprint_opt_inputslot"));
			}
		}
	}
}

void KPdqView::setOptions(const OptionSet& opts)
{
	if (root_) root_->setOptions(opts);
}

void KPdqView::getOptions(OptionSet& opts, bool incldef)
{
	if (root_) root_->getOptions(opts,incldef);
}

void KPdqView::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	//setColumnWidth(0,width());
}
