#ifndef DRIVERITEM_H
#define DRIVERITEM_H

#include <qlistview.h>

class DrBase;

class DriverItem : public QListViewItem
{
public:
	DriverItem(QListView *parent, DrBase *item);
	DriverItem(QListViewItem *parent, DrBase *item);

	void updateText();
	DrBase* drItem() const 	{ return m_item; }
	bool hasConflict()	{ return m_conflict; }
	bool updateConflict();

	void paintCell(QPainter*, const QColorGroup&, int, int, int);

private:
	DrBase	*m_item;
	bool	m_conflict;
};

#endif
