#include "kppdoptionitem.h"
#include "icons.h"

#include <qpainter.h>
#include <kiconloader.h>
#include <klocale.h>

KPPDOptionItem::KPPDOptionItem(global_ppd_file_t *ppd, ppd_option_t *option, QListViewItem *parent, bool changeable)
	: KPPDBaseItem(ppd,parent)
{
	type_ = Regular;
	option_ = option;
	QString	str = getOptionIconName(option_->keyword);
	setPixmap(0,UserIcon((str.isEmpty() ? QString::fromLatin1("kdeprint_opt_list") : str)));
	currentchoice_ = 0;
	changeable_ = changeable;
        ppd_choice_t	*defchoice_(0);
	for (int i=0;i<option_->num_choices;i++)
	{
		ppd_choice_t	*ch = option_->choices+i;
		if (ch->marked) currentchoice_ = ch;
		if (strcmp(ch->choice,option_->defchoice) == 0) defchoice_ = ch;
	}
	setCurrentChoice((currentchoice_ ? currentchoice_ : defchoice_));
	updateText();
}

KPPDOptionItem::~KPPDOptionItem()
{
}

void KPPDOptionItem::setCurrentChoice(int index)
{
	if (index >= 0 && index < option_->num_choices)
		setCurrentChoice(option_->choices+index);
	else
		setCurrentChoice(0);
}

void KPPDOptionItem::setCurrentChoice(ppd_choice_t *ch)
{
	currentchoice_ = ch;
	if (option_->ui == PPD_UI_BOOLEAN)
	{
		if (!currentchoice_) setPixmap(0,UserIcon(""));
		else if (strcmp(currentchoice_->choice,"True") == 0) setPixmap(0,UserIcon("kdeprint_bool_on"));
		else setPixmap(0,UserIcon("kdeprint_bool_off"));
	}
	if (currentchoice_ && ppd_) globalPpdMarkOption(ppd_,option_->keyword,currentchoice_->choice);
	updateText();
}

void KPPDOptionItem::updateText()
{
	QString	txt = QString("%1: <%2>").arg(QString::fromLocal8Bit(option_->text)).arg((currentchoice_ ? QString::fromLocal8Bit(currentchoice_->text) : i18n("Undefined")));
	setText(0,txt);
}

void KPPDOptionItem::markChoices(global_ppd_file_t *ppd)
{
	if (currentchoice_)
		globalPpdMarkOption(ppd,option_->keyword,currentchoice_->choice);
}

void KPPDOptionItem::updateChoices(global_ppd_file_t *ppd)
{
	ppd_choice_t	*ch = ppdFindMarkedChoice(ppd->ppd,option_->keyword);
	setCurrentChoice(ch);
}

void KPPDOptionItem::getOptions(OptionSet& opts, bool incldef)
{
	if (currentchoice_ && (incldef || strcmp(currentchoice_->choice,option_->defchoice) != 0)) {
		opts[option_->keyword] = currentchoice_->choice;
	}
}

void KPPDOptionItem::setOptions(const OptionSet& opts)
{
	QString	value = opts[option_->keyword];
	if (!value.isEmpty())
	{
		ppd_choice_t	*ch = ppdFindChoice(option_,value.latin1());
		if (ch) setCurrentChoice(ch);
	}
}

void KPPDOptionItem::paintText(QPainter *p, const QColorGroup& cg, int r)
{
	QString	txt;
	QFontMetrics	fm(p->fontMetrics());

	// first part (normal color)
	txt = QString::fromLocal8Bit(option_->text) + ": <";
	p->setPen(isSelected() ? white : cg.text());
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
	r += fm.width(txt);
	// second part (option color)
	if (currentchoice_)
	{
		txt = QString::fromLocal8Bit(currentchoice_->text);
		if (!isSelected()) p->setPen((conflict_ ? red : darkGreen));
	}
	else
		txt = i18n("Undefined");
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
	r += fm.width(txt);
	// last part (normal color)
	p->setPen(isSelected() ? white : cg.text());
	txt = QString::fromLatin1(">");
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
}

void KPPDOptionItem::setupDict(QDict<KPPDBaseItem>& dict) const
{
	dict.insert(option_->keyword,this);
}

void KPPDOptionItem::updateConflict()
{
	if (option_->conflicted) setConflict(true);
}
