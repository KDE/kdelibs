#ifndef KPDQARGITEM_H
#define KPDQARGITEM_H

#include "kpdqbaseitem.h"

class PDQArgument;
class PDQBase;

class KPdqArgItem : public KPdqBaseItem
{
friend class KPdqOptionView;
public:
	KPdqArgItem(PDQArgument *arg, QListViewItem *parent, QListViewItem *after, const QString& name);
	~KPdqArgItem();

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);
	PDQBase* pdqItem();
	void setValue(const QString& txt);

	void updateText();

private:
	PDQArgument	*argument_;
	QString		value_;
};

#endif
