#include "kpdqbaseitem.h"

KPdqBaseItem::KPdqBaseItem(QListViewItem *parent, QListViewItem *after, const QString& name)
: QListViewItem(parent,after,name)
{
	type_ = KPdqBaseItem::Base;
	setOpen(true);
}

KPdqBaseItem::KPdqBaseItem(QListView *parent, const QString& name)
: QListViewItem(parent,name)
{
	type_ = KPdqBaseItem::Base;
	setOpen(true);
}

KPdqBaseItem::~KPdqBaseItem()
{
}

void KPdqBaseItem::setOptions(const OptionSet& opts)
{
	KPdqBaseItem	*item = (KPdqBaseItem*)(firstChild());
	while (item)
	{
		item->setOptions(opts);
		item = (KPdqBaseItem*)(item->nextSibling());
	}
}

void KPdqBaseItem::getOptions(OptionSet& opts, bool incldef)
{
	KPdqBaseItem	*item = (KPdqBaseItem*)(firstChild());
	while (item)
	{
		item->getOptions(opts,incldef);
		item = (KPdqBaseItem*)(item->nextSibling());
	}
}

PDQBase* KPdqBaseItem::pdqItem()
{
	return 0;
}
