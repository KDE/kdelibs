#ifndef KPPDOPTIONITEM_H
#define	KPPDOPTIONITEM_H

#include "kppdbaseitem.h"

class KPPDOptionItem : public KPPDBaseItem
{
public:
	KPPDOptionItem(global_ppd_file_t *ppd, ppd_option_t *option, QListViewItem *parent, bool changeable = true);
	~KPPDOptionItem();

	void updateText();
	void markChoices(global_ppd_file_t *ppd);
	void updateChoices(global_ppd_file_t *ppd);
	void getOptions(OptionSet& opts, bool incldef = false);
	void setOptions(const OptionSet& opts);
	void setupDict(QDict<KPPDBaseItem>& dict) const;
	void setCurrentChoice(int index);
	void setCurrentChoice(ppd_choice_t *ch);
	void updateConflict();

	virtual void paintText(QPainter *p, const QColorGroup& cg, int r);

public:
	ppd_option_t	*option_;
	ppd_choice_t	*currentchoice_;
	bool		changeable_;
};

#endif
