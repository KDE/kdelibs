#include "kppdnumericaloptionitem.h"
#include "icons.h"

#include <kiconloader.h>

KPPDNumericalOptionItem::KPPDNumericalOptionItem(global_ppd_file_t *ppd, numerical_opt_t *option, QListViewItem *parent)
	: KPPDBaseItem(ppd,parent)
{
	type_ = Numeric;
	option_ = option;
	value_ = option_->defvalue;
	updateText();

	QString	str = getOptionIconName(option_->keyword);
	setPixmap(0,UserIcon((str.isEmpty() ? QString::fromLatin1("kdeprint_opt_numeric") : str)));
}

KPPDNumericalOptionItem::~KPPDNumericalOptionItem()
{
}

void KPPDNumericalOptionItem::updateText()
{
	QString	txt = QString("%1: <").arg(QString::fromLocal8Bit(option_->text));
	if (option_->type == 0) txt += QString("%1>").arg((int)(value_));
	else txt += QString("%1>").arg(value_,0,'f',3);
	setText(0,txt);
}

void KPPDNumericalOptionItem::markChoices(global_ppd_file_t *ppd)
{
	globalPpdMarkNumericalOption(ppd,option_->keyword,value_);
}

void KPPDNumericalOptionItem::updateChoices(global_ppd_file_t*)
{
	value_ = option_->value;
	updateText();
}

void KPPDNumericalOptionItem::getOptions(OptionSet& opts, bool incldef)
{
	if (incldef || value_ != option_->defvalue)
	{
		QString	valstr;
		if (option_->type == 0) valstr.sprintf("%d",(int)value_);
		else valstr.sprintf("%.3f",value_);
		opts[option_->keyword] = valstr;
	}
}

void KPPDNumericalOptionItem::setOptions(const OptionSet& opts)
{
	QString	value = opts[option_->keyword];
	if (!value.isEmpty())
	{
		bool	ok;
		double	val;
		if (option_->type == 0) val = (double)value.toInt(&ok);
		else val = (double)value.toFloat(&ok);
		if (ok)
			setValue(val);
	}
}

void KPPDNumericalOptionItem::paintText(QPainter *p, const QColorGroup& cg, int r)
{
	QString	txt;
	QFontMetrics	fm(p->fontMetrics());

	// first part (normal color)
	txt = QString("%1: <").arg(QString::fromLocal8Bit(option_->text));
	p->setPen(isSelected() ? white : cg.text());
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
	r += fm.width(txt);
	// second part (option color)
	p->setPen(isSelected() ? white : darkGreen);
	if (option_->type == 0) txt.sprintf("%d",(int)value_);
	else txt.sprintf("%.3f",value_);
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
	r += fm.width(txt);
	// last part (normal color)
	p->setPen(isSelected() ? white : cg.text());
	txt = QString::fromLatin1(">");
	p->drawText(r,0,fm.width(txt),height(),AlignLeft|AlignVCenter,txt);
}

void KPPDNumericalOptionItem::setValue(double value)
{
	value_ = value;
	option_->value = value;
	updateText();
}
