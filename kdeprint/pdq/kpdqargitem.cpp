#include "kpdqargitem.h"
#include "pdqclass.h"

#include <klocale.h>

KPdqArgItem::KPdqArgItem(PDQArgument *arg, QListViewItem *parent, QListViewItem *after, const QString& name)
: KPdqBaseItem(parent,after,name)
{
	setType(KPdqBaseItem::Arg);

	argument_ = arg;
	value_ = QString::null;

	if (argument_)
	{
		setText(0,argument_->description_);
		value_ = argument_->defvalue_;
		setToolTip(argument_->help_);
	}

	updateText();
}

KPdqArgItem::~KPdqArgItem()
{
}

void KPdqArgItem::updateText()
{
	if (argument_)
	{
		QString	val = QString::fromLatin1("%1: <%2>").arg(argument_->description_).arg((value_.isEmpty() ? i18n("Undefined") : value_));
		setText(0,val);
		widthChanged();
	}
}

void KPdqArgItem::setOptions(const OptionSet& opts)
{
	if (argument_)
	{
		QString	value = opts[argument_->name()];
		if (!value.isEmpty())
			setValue(value);
	}
}

void KPdqArgItem::getOptions(OptionSet& opts, bool incldef)
{
	if (argument_)
	{
		if (value_ != argument_->defvalue_ || incldef)
			opts[argument_->name()] = value_;
	}
}

void KPdqArgItem::setValue(const QString& txt)
{
	value_ = txt;
	updateText();
}

PDQBase* KPdqArgItem::pdqItem()
{
	return argument_;
}
