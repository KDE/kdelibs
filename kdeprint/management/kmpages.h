#ifndef KMPAGES_H
#define KMPAGES_H

#include <qtabwidget.h>
#include <qlist.h>

class KMPrinter;
class KMPrinterPage;

class KMPages : public QTabWidget
{
public:
	KMPages(QWidget *parent = 0, const char *name = 0);
	~KMPages();

	void setPrinter(KMPrinter *p);

protected:
	void initialize();

private:
	QList<KMPrinterPage>	m_pages;
};

#endif
