#ifndef KMWIZARDPAGE_H
#define KMWIZARDPAGE_H

#include <qwidget.h>

class KMPrinter;

class KMWizardPage : public QWidget
{
	Q_OBJECT
public:
	KMWizardPage(QWidget *parent = 0, const char *name = 0);
	virtual ~KMWizardPage();

	int id() const 			{ return m_ID; }
	const QString& title() const 	{ return m_title; }
	int nextPage() const 		{ return m_nextpage; }
	void setNextPage(int p)		{ m_nextpage = p; }
	virtual bool isValid(QString&);
	virtual void initPrinter(KMPrinter*);
	virtual void updatePrinter(KMPrinter*);

protected:
	QString	m_title;
	int	m_ID;
	int	m_nextpage;
};

#endif
