#include "kppdview.h"
#include "kppdbaseitem.h"
#include "kppdgroupitem.h"
#include "kppdoptionitem.h"
#include "kppdnumericaloptionitem.h"
#include "ppd-util.h"

#include <qheader.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>

KPPDView::KPPDView(global_ppd_file_t *ppd, QWidget *parent, const char *name)
	: QListView(parent,name)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding));

	addColumn( i18n("Options") );
	header()->hide();
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);

	rootitem_ = 0;
	ppd_ = 0;
	options_.setAutoDelete(false);
	conflict_ = false;
	if (ppd) initialize(ppd);
}

KPPDView::~KPPDView()
{
}

void KPPDView::initialize(global_ppd_file_t *ppd)
{
	ppd_ = ppd;
	clear();
	if (!ppd_) return;

	options_.clear();

	setSorting(-1);

	rootitem_ = new KPPDBaseItem(ppd_,this);
	rootitem_->setText(0,ppd->ppd->nickname);
	rootitem_->setOpen(true);
	rootitem_->setPixmap(0,UserIcon("kdeprint_printer"));

	// add numerical options
	if (ppd_->num_numerical_options > 0)
	{
		KPPDBaseItem	*numericalitem;
		numericalitem = new KPPDBaseItem(ppd_,rootitem_);
		numericalitem->setText(0, i18n("Adjustments") );
		numericalitem->setOpen(true);
		numericalitem->setPixmap(0,UserIcon("kdeprint_group_adjust"));
		for (int i=ppd_->num_numerical_options-1;i>=0;i--)
		{
			numerical_opt_t	*opt = ppd_->numerical_options+i;
			new KPPDNumericalOptionItem(ppd_,opt,numericalitem);
		}
	}

	// add regular options
	for (int i=ppd_->ppd->num_groups-1;i>=0;i--)
	{
		ppd_group_t	*group = ppd_->ppd->groups+i;
		new KPPDGroupItem(ppd_,group,rootitem_);
	}
	rootitem_->setupDict(options_);

	//checkConstraints();
	checkConflict();
}

void KPPDView::markChoices(global_ppd_file_t *ppd)
{
	if (rootitem_) rootitem_->markChoices(ppd);
}

void KPPDView::updateChoices(global_ppd_file_t *ppd)
{
	if (rootitem_) rootitem_->updateChoices(ppd);
	checkConflict();
}

void KPPDView::getOptions(OptionSet& opts, bool incldef)
{
	if (rootitem_) rootitem_->getOptions(opts,incldef);
}

void KPPDView::setOptions(const OptionSet& opts)
{
debug("***********************");
debug("updating PPD widget...");
opts.dump();
	if (rootitem_) rootitem_->setOptions(opts);
	emit selectionChanged(currentItem());
	checkConstraints();
}

void KPPDView::checkConstraints()
{
	checkConflict();
	if (conflict_)
	{
		QString	msg = ppdConflictErrorMsg(ppd_->ppd);
		if (!msg.isEmpty()) QMessageBox::warning(this,i18n("Driver configuration"),msg,QMessageBox::Ok|QMessageBox::Default,0);
	}
}

void KPPDView::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	setColumnWidth(0,width());
}

bool KPPDView::hasConflict()
{
	checkConflict();
        return conflict_;
}

void KPPDView::checkConflict()
{
	if (!rootitem_ || !ppd_) return;

	// reset all options
	rootitem_->setConflict(false);
	conflict_ = false;

	// update conflict
	conflict_ = (ppdConflicts(ppd_->ppd) > 0);

	// updates conflicting options
	if (conflict_)
	{
		QDictIterator<KPPDBaseItem>	oit(options_);
		for (;oit.current();++oit) oit.current()->updateConflict();
	}
}

//--------------------------------------------------------------------------------

int parseOption(ppd_option_t *opt, QString& msg)
{
	if (opt->conflicted)
	{
		ppd_choice_t	*ch(0);
		for (int i=0;i<opt->num_choices;i++)
		{
			ch = opt->choices+i;
			if (ch->marked) break;
			else ch = 0;
		}
		if (ch)
		{
			QString	msg2 = QString("  %1 = %2\n").arg(QString::fromLocal8Bit(opt->text)).arg(QString::fromLocal8Bit(ch->text));
			msg.append(msg2);
			return 1;
		}
	}
	return 0;
}

int parseGroup(ppd_group_t *gr, QString& msg)
{
	int	n(0);
	for (int i=0;i<gr->num_subgroups;i++) n += parseGroup(gr->subgroups+i,msg);
	for (int i=0;i<gr->num_options;i++) n += parseOption(gr->options+i,msg);
	return n;
}

QString ppdConflictErrorMsg(ppd_file_t *ppd)
{
	QString	msg(i18n("You selected options that are in conflict. Try to resolve the\nproblem before continuing:\n\n"));
	int	n(0);
	for (int i=0;i<ppd->num_groups;i++) n += parseGroup(ppd->groups+i,msg);
	return (n == 0 ? QString("") : msg);
}
