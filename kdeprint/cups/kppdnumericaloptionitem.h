#ifndef	KPPDNUMERICALOPTIONITEM_H
#define	KPPDNUMERICALOPTIONITEM_H

#include "kppdbaseitem.h"

class KPPDNumericalOptionItem : public KPPDBaseItem
{
public:
	KPPDNumericalOptionItem(global_ppd_file_t *ppd, numerical_opt_t *option, QListViewItem *parent);
	~KPPDNumericalOptionItem();

	void updateText();
	void markChoices(global_ppd_file_t *ppd);
	void updateChoices(global_ppd_file_t *ppd);
	void getOptions(OptionSet& opts, bool incldef = false);
	void setOptions(const OptionSet& opts);
	void setValue(double value);

	virtual void paintText(QPainter *p, const QColorGroup& cg, int r);

public:
	numerical_opt_t	*option_;
	double	value_;
};

#endif
