#ifndef KPDQOPTIONSITEM_H
#define KPDQOPTIONSITEM_H

#include "kpdqbaseitem.h"

#include <qlist.h>

class PDQOption;
class PDQChoice;
class PDQBase;

class KPdqOptionItem : public KPdqBaseItem
{
friend class KPdqOptionView;
public:
	KPdqOptionItem(PDQOption *opt, QListViewItem *parent, QListViewItem *after, const QString& name);
	~KPdqOptionItem();

	void setOptions(const OptionSet& opts);
	void getOptions(OptionSet& opts, bool incldef = false);
	PDQBase* pdqItem();
	void setCurrentChoice(int index);
	void setCurrentChoice(PDQChoice *ch);

	void updateText();

private:
	PDQOption	*option_;
	PDQChoice	*currentchoice_;
};

#endif
