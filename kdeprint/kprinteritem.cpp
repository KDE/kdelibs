#include "kprinteritem.h"

KPrinterItem::KPrinterItem()
{
	discarded_ = false;
	type_ = 0;
	default_ = false;
	edited_ = false;
	state_ = KPrinterItem::Idle;
}

void KPrinterItem::copy(const KPrinterItem& o)
{
	discarded_ = o.discarded_;

	displayname_ = o.displayname_;
	printername_ = o.printername_;
	name_ = o.name_;
	type_ = o.type_;
	pixmap_ = o.pixmap_;
	default_ = o.default_;
	state_ = o.state_;
	// do not copy the "edited" state, as it defaults to "false"

	for (QMap<QString,QString>::ConstIterator it = o.defaultset_.begin(); it != o.defaultset_.end(); ++it)
		defaultset_[it.key()] = (*it);
	for (QMap<QString,QString>::ConstIterator it = o.editedset_.begin(); it != o.editedset_.end(); ++it)
		editedset_[it.key()] = (*it);
}

void KPrinterItem::clear()
{
	defaultset_.clear();
	editedset_.clear();
}
