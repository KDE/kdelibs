#ifndef KMPRINTERVIEW_H
#define KMPRINTERVIEW_H

#include <qwidget.h>
#include <qlist.h>

class KMIconView;
class KMListView;
class KMPrinter;
class QIconViewItem;

class KMPrinterView : public QWidget
{
	Q_OBJECT
public:
	enum ViewType { Icons = 0, List, Tree };

	KMPrinterView(QWidget *parent = 0, const char *name = 0);
	~KMPrinterView();

	void setPrinterList(QList<KMPrinter> *list);
	void setViewType(ViewType t);

signals:
	void printerSelected(KMPrinter*);
	void rightButtonClicked(KMPrinter*, const QPoint&);

protected slots:
	void slotPrinterSelected(KMPrinter*);

private:
	KMIconView		*m_iconview;
	KMListView		*m_listview;
	ViewType		m_type;
	QList<KMPrinter>	*m_printers;
	KMPrinter		*m_current;
};

#endif
