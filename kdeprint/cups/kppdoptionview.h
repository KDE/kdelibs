#ifndef KPPDOPTIONVIEW_H
#define	KPPDOPTIONVIEW_H

#include <qgroupbox.h>

class QListBox;
class QButtonGroup;
class QWidgetStack;
class NumericWidget;
class QListViewItem;
class KPPDOptionItem;
class KPPDNumericalOptionItem;
class KPPDBaseItem;

class KPPDOptionView : public QGroupBox
{
	Q_OBJECT;
public:
	enum ViewType {List, Radio, Numeric};
	KPPDOptionView(QWidget *parent = 0, const char *name = 0);
	~KPPDOptionView();
	void setAllowNonChangeable(bool on = true) { allowNonChangeable_ = on; }
	bool allowNonChangeable() const { return allowNonChangeable_; }
	QSize sizeHint() const;
signals:
	void optionChanged();
public slots:
	void setOption(QListViewItem *item);
	void selectionChanged(int index);
	void valueChanged(double value);
private:
	QWidgetStack	*stack_;
	QListBox	*optionpickone_;
	QButtonGroup	*optionboolean_;
	NumericWidget	*optionnumeric_;
	QWidget	*optionempty_;
	KPPDBaseItem	*ppdopt_;
	bool		allowNonChangeable_;
	bool		block_;
};

#endif
