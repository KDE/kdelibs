#ifndef	KPPDBASEITEM_H
#define	KPPDBASEITEM_H

#include <qlistview.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qdict.h>

#include "globalppd.h"

#include "optionset.h"

class KPPDBaseItem : public QListViewItem
{
public:
	enum OptionType { Regular, Numeric };

	KPPDBaseItem(global_ppd_file_t *ppd, QListView *parent);
	KPPDBaseItem(global_ppd_file_t *ppd, QListViewItem *parent);
	KPPDBaseItem(global_ppd_file_t *ppd, QListViewItem *parent, QListViewItem *after);
	~KPPDBaseItem();

	virtual void setConflict(bool on = true);
	virtual void paintCell(QPainter *P, const QColorGroup& cg, int column, int width, int align);
	virtual int textWidth(QPainter *p) const;
	virtual void paintText(QPainter *p, const QColorGroup& cg, int r);
	virtual void setupDict(QDict<KPPDBaseItem>& dict) const;
	bool isGroup() const { return !children_.isEmpty(); }
	bool isOption() const { return (children_.isEmpty() && depth()>1 && type_ == Regular); }
	bool isNumeric() const { return (children_.isEmpty() && depth()>1 && type_ == Numeric); }
	bool showConflict() const { return (conflict_ && (isOption() || (isGroup() && !isOpen()))); }
	virtual void updateConflict() {}

	virtual void markChoices(global_ppd_file_t *ppd);
	virtual void updateChoices(global_ppd_file_t *ppd);
	virtual void getOptions(OptionSet& opts, bool incldef = false);
	virtual void setOptions(const OptionSet& opts);

public:
	bool	conflict_;
	QList<KPPDBaseItem>	children_;
	global_ppd_file_t	*ppd_;
	int			type_;
};

#endif
