#ifndef KPDQBASEITEM_H
#define KPDQBASEITEM_H

#include <qlistview.h>

#include "optionset.h"

class PDQBase;

class KPdqBaseItem : public QListViewItem
{
public:
	enum ItemType { Base = 0, Option = 1, Arg = 2};

	KPdqBaseItem(QListViewItem *parent, QListViewItem *after, const QString& name);
	KPdqBaseItem(QListView *parent, const QString& name);
	~KPdqBaseItem();

	virtual void setOptions(const OptionSet& opts);
	virtual void getOptions(OptionSet& opts, bool incldef = false);
	virtual PDQBase* pdqItem();

	ItemType type() const;
	void setType(ItemType);
	QString toolTip() const;
	void setToolTip(const QString&);

private:
	ItemType	type_;
	QString		tip_;
};

inline KPdqBaseItem::ItemType KPdqBaseItem::type() const
{ return type_; }

inline void KPdqBaseItem::setType(ItemType t)
{ type_ = t; }

inline QString KPdqBaseItem::toolTip() const
{ return tip_; }

inline void KPdqBaseItem::setToolTip(const QString& s)
{ if (!s.isEmpty()) tip_ = QString::fromLatin1("<p>%1</p>").arg(s); else tip_ = QString::null; }

#endif
