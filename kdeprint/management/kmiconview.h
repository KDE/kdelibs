#ifndef KMICONVIEW_H
#define KMICONVIEW_H

#include <kiconview.h>
#include <qlist.h>

#include "kmobject.h"

class KMPrinter;

class KMIconViewItem : public QIconViewItem, public KMObject
{
public:
	KMIconViewItem(QIconView *parent, KMPrinter *p);
	KMPrinter* printer() const	{ return m_printer; }
	void updatePixmap(int pos = -1);
	void updatePrinter(KMPrinter*);

protected:
	virtual void paintItem(QPainter*, const QColorGroup&);

private:
	KMPrinter	*m_printer;
	int		m_size;
	QString		m_pixmap;
	char		m_state;
};

class KMIconView : public KIconView
{
	Q_OBJECT
public:
	enum ViewMode { Big, Small };

	KMIconView(QWidget *parent = 0, const char *name = 0);
	~KMIconView();

	void setPrinterList(QList<KMPrinter> *list);
	void setPrinter(KMPrinter*);
	void setViewMode(ViewMode);

signals:
	void rightButtonClicked(KMPrinter*, const QPoint&);
	void printerSelected(KMPrinter*);

protected slots:
	void slotRightButtonClicked(QIconViewItem*, const QPoint&);
	void slotSelectionChanged();

private:
	KMIconViewItem* findItem(KMPrinter *p);

private:
	QList<KMIconViewItem>	m_items;
	ViewMode		m_mode;
};

#endif
