#ifndef KPDQOPTIONVIEW_H
#define KPDQOPTIONVIEW_H

#include <qgroupbox.h>
#include <qlist.h>

class QLineEdit;
class QWidgetStack;
class QListBox;
class KPdqBaseItem;
class QListViewItem;

class KPdqOptionView : public QGroupBox
{
	Q_OBJECT
public:
	KPdqOptionView(QWidget *parent = 0, const char *name = 0);
	~KPdqOptionView();

	QSize sizeHint() const;

public slots:
	void optionSelected(QListViewItem*);
	void valueChanged(int);
	void valueChanged(const QString& txt);

private:
	QWidgetStack	*stack_;
	QLineEdit	*edit_;
	QListBox	*list_;
	QList<QWidget>	widgetlist_;
	KPdqBaseItem	*item_;
};

#endif
