#ifndef CJANUSWIDGET_H
#define CJANUSWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qlist.h>

class QWidgetStack;
class QLabel;
class QListBoxItem;

class CJanusWidget : public QWidget
{
	Q_OBJECT
public:
	CJanusWidget(QWidget *parent, const char *name);
	~CJanusWidget();

	void addPage(QWidget *w, const QString& text, const QString& header, const QPixmap& pix);
	void enablePage(QWidget *w);
	void disablePage(QWidget *w);

protected slots:
	void slotSelected(QListBoxItem*);

private:
	class CListBox;
	class CListBoxItem;
	struct CPage;
	CPage* findPage(QWidget *w);
	CPage* findPage(QListBoxItem *i);
	QListBoxItem* findPrevItem(CPage*);

private:
	QList<CPage>		m_pages;
	CListBox		*m_iconlist;
	QLabel			*m_header;
	QWidgetStack		*m_stack;
	QWidget			*m_empty;
};

#endif
