#ifndef KMLISTVIEW_H
#define KMLISTVIEW_H

#include <qlistview.h>
#include <qlist.h>

class KMListViewItem;
class KMPrinter;

class KMListView : public QListView
{
	Q_OBJECT
public:
	KMListView(QWidget *parent = 0, const char *name = 0);
	~KMListView();

	void setPrinterList(QList<KMPrinter> *list);
	void setPrinter(KMPrinter*);

signals:
	void rightButtonClicked(KMPrinter*, const QPoint&);
	void printerSelected(KMPrinter*);

protected slots:
	void slotRightButtonClicked(QListViewItem*, const QPoint&, int);
	void slotSelectionChanged();
	void slotOnItem(QListViewItem*);
	void slotOnViewport();

protected:
	void resizeEvent(QResizeEvent*);
	KMListViewItem* findItem(KMPrinter*);

private:
	QList<KMListViewItem>	m_items;
	KMListViewItem		*m_root, *m_classes, *m_printers;
};

#endif
