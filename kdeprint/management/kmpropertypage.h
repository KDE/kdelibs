#ifndef KMPROPERTYPAGE_H
#define KMPROPERTYPAGE_H

#include "cjanuswidget.h"
#include "kmprinterpage.h"

#include <qlist.h>

class KMPropWidget;
class KMPrinter;

class KMPropertyPage : public CJanusWidget, public KMPrinterPage
{
	Q_OBJECT
public:
	KMPropertyPage(QWidget *parent = 0, const char *name = 0);
	~KMPropertyPage();

	void addPropPage(KMPropWidget*);
	void setPrinter(KMPrinter*);

protected slots:
	void slotEnable(bool);

private:
	QList<KMPropWidget>	m_widgets;
};

#endif
