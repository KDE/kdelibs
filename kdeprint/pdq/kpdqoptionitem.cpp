#include "kpdqoptionitem.h"
#include "pdqclass.h"

#include <klocale.h>

KPdqOptionItem::KPdqOptionItem(PDQOption *opt, QListViewItem *parent, QListViewItem *after, const QString& name)
: KPdqBaseItem(parent,after,name)
{
	setType(KPdqBaseItem::Option);

	option_ = opt;
	currentchoice_ = 0;

	if (option_)
	{
		QListIterator<PDQChoice>	it(option_->choices_);
		currentchoice_ = it.current();	// set current choice to first choice by default
		for (;it.current();++it)
			if (it.current()->name() == option_->default_choice_) currentchoice_ = it.current();
	}

	updateText();
}

KPdqOptionItem::~KPdqOptionItem()
{
}

void KPdqOptionItem::updateText()
{
	if (option_)
	{
		QString	val = QString::fromLatin1("%1: <%2>").arg(option_->description_).arg((currentchoice_ ? currentchoice_->description_ : i18n("Undefined")));
		setText(0,val);
		widthChanged();
		setToolTip((currentchoice_ ? currentchoice_->help_ : QString::null));
	}
}

void KPdqOptionItem::setOptions(const OptionSet& opts)
{
	if (option_)
	{
		QString	value = opts[option_->name()];
		if (!value.isEmpty())
		{
			QListIterator<PDQChoice>	it(option_->choices_);
			for (;it.current();++it)
				if (it.current()->name() == value)
				{
					currentchoice_ = it.current();
					updateText();
					break;
				}
		}
	}
	KPdqBaseItem::setOptions(opts);
}

void KPdqOptionItem::getOptions(OptionSet& opts, bool incldef)
{
	if (option_ && currentchoice_)
	{
		if (currentchoice_->name() != option_->default_choice_ || incldef)
			opts[option_->name()] = currentchoice_->name();
	}
	KPdqBaseItem::getOptions(opts,incldef);
}

void KPdqOptionItem::setCurrentChoice(int index)
{
	if (index < 0 || index >= (int)(option_->choices_.count())) return;
	setCurrentChoice(option_->choices_.at(index));
}

void KPdqOptionItem::setCurrentChoice(PDQChoice *ch)
{
	currentchoice_ = ch;
	updateText();
}

PDQBase* KPdqOptionItem::pdqItem()
{
	return option_;
}
