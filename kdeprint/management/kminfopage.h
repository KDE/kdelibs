#ifndef KMINFOPAGE_H
#define KMINFOPAGE_H

#include "kmprinterpage.h"
#include <qwidget.h>

class QLabel;

class KMInfoPage : public QWidget, public KMPrinterPage
{
public:
	KMInfoPage(QWidget *parent = 0, const char *name = 0);
	~KMInfoPage();

	void setPrinter(KMPrinter *p);

protected:
	QLabel	*m_title, *m_titlepixmap;
	QLabel	*m_location, *m_description, *m_uri, *m_model, *m_type, *m_state, *m_device;
	QLabel	*m_devlabel;
};

#endif
