#ifndef KMPROPCONTAINER_H
#define KMPROPCONTAINER_H

#include <qwidget.h>

class KMPropWidget;
class QPushButton;
class KMPrinter;

class KMPropContainer : public QWidget
{
	Q_OBJECT
public:
	KMPropContainer(QWidget *parent = 0, const char *name = 0);
	~KMPropContainer();

	void setWidget(KMPropWidget*);
	void setPrinter(KMPrinter*);

signals:
	void enable(bool);

private:
	KMPropWidget	*m_widget;
	QPushButton	*m_button;
};

#endif
