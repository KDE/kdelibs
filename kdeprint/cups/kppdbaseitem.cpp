#include "kppdbaseitem.h"

#include <qpainter.h>
#include <qapplication.h>

KPPDBaseItem::KPPDBaseItem(global_ppd_file_t *ppd, QListView *parent)
	: QListViewItem(parent)
{
	children_.setAutoDelete(false);
	ppd_ = ppd;
	type_ = Regular;
}

KPPDBaseItem::KPPDBaseItem(global_ppd_file_t *ppd, QListViewItem *parent)
	: QListViewItem(parent)
{
	children_.setAutoDelete(false);
	((KPPDBaseItem*)parent)->children_.append(this);
	ppd_ = ppd;
	type_ = Regular;
}

KPPDBaseItem::KPPDBaseItem(global_ppd_file_t *ppd, QListViewItem *parent, QListViewItem *after)
	: QListViewItem(parent,after)
{
	children_.setAutoDelete(false);
	((KPPDBaseItem*)parent)->children_.append(this);
	ppd_ = ppd;
	type_ = Regular;
}

KPPDBaseItem::~KPPDBaseItem()
{
}

int KPPDBaseItem::textWidth(QPainter *p) const
{
	return p->fontMetrics().width(text(0));
}

void KPPDBaseItem::setConflict(bool on)
{
	conflict_ = on;
	if (conflict_)
	{
		if (depth() > 0) ((KPPDBaseItem*)parent())->setConflict(on);
	}
	else
	{
		QListIterator<KPPDBaseItem>	it(children_);
		for (;it.current();++it) it.current()->setConflict(on);
	}
	repaint();
}

void KPPDBaseItem::paintText(QPainter *p, const QColorGroup& cg, int r)
{
	p->setPen(isSelected() ? white : (showConflict() ? red : cg.text()));
	p->drawText(r,0,textWidth(p),height(),AlignVCenter|AlignLeft,text(0));
}

void KPPDBaseItem::paintCell(QPainter *p, const QColorGroup& cg, int column, int width, int)
{
    QListView *lv = listView();
    int r = lv->itemMargin();
    const QPixmap * icon = pixmap( column );

    p->fillRect( 0, 0, width, height(), cg.base() );

    int marg = r;
    int	w = textWidth(p) + 2 * marg;

    if (icon) w += (icon->width() + marg);

    if ( isSelected() ) {
	    p->fillRect( r - marg, 0, w, height(),
			 (showConflict() ? red : QApplication::winStyleHighlightColor()) );
    }

    if ( icon ) {
	p->drawPixmap( r, (height()-icon->height())/2, *icon );
	r += icon->width() + marg;
    }

    paintText( p, cg, r );
}

void KPPDBaseItem::markChoices(global_ppd_file_t *ppd)
{
	QListIterator<KPPDBaseItem>	it(children_);
	for (;it.current();++it) it.current()->markChoices(ppd);
}

void KPPDBaseItem::updateChoices(global_ppd_file_t *ppd)
{
	QListIterator<KPPDBaseItem>	it(children_);
	for (;it.current();++it) it.current()->updateChoices(ppd);
}

void KPPDBaseItem::getOptions(OptionSet& opts, bool incldef)
{
	QListIterator<KPPDBaseItem>	it(children_);
	for (;it.current();++it) it.current()->getOptions(opts,incldef);
}

void KPPDBaseItem::setOptions(const OptionSet& opts)
{
	QListIterator<KPPDBaseItem>	it(children_);
	for (;it.current();++it) it.current()->setOptions(opts);
}

void KPPDBaseItem::setupDict(QDict<KPPDBaseItem>& dict) const
{
	QListIterator<KPPDBaseItem>	it(children_);
	for (;it.current();++it) it.current()->setupDict(dict);
}
