#ifndef KMWIZARD_H
#define KMWIZARD_H

#include <qdialog.h>
#include <qintdict.h>
#include <qvaluestack.h>

class QWidgetStack;
class QLabel;
class QPushButton;
class KMWizardPage;
class KMPrinter;
class KMWBackend;

class KMWizard : public QDialog
{
	Q_OBJECT
public:
	enum PageType {
		Start   = 0,
		End,
		Error,
		Backend,
		Driver,
		File,
		SMB,
		TCP,
		Local,
		LPD,
		IPP,
		IPPSelect,
		Class,
		Password,
		DriverTest,
		DriverSelect,
		Name,
		Banners,
		Custom  = 100
	};

	KMWizard(QWidget *parent = 0, const char *name = 0);
	~KMWizard();

	void configure(int start, int end, bool inclusive = true);
	void setCurrentPage(int ID, bool back = false);
	void setPrinter(KMPrinter*);
	KMPrinter* printer()		{ return m_printer; }
	void addPage(KMWizardPage*);
	KMWBackend* backendPage()	{ return m_backend; }
	void setNextPage(int page, int next);

protected slots:
	void slotNext();
	void slotPrev();

private:
	QIntDict<KMWizardPage>	m_pagepool;
	QValueStack<int>	m_pagestack;

	QWidgetStack		*m_stack;
	QLabel			*m_title;
	QPushButton		*m_next, *m_prev;
	int			m_start, m_end;
	bool			m_inclusive;
	KMPrinter		*m_printer;

	// backend page
	KMWBackend		*m_backend;
};

#endif
