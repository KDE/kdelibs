#ifndef KMUIMANAGER_H
#define KMUIMANAGER_H

#include <qobject.h>
#include <qlist.h>

#include "kprinter.h"

class KMPropertyPage;
class KMWizard;
class KPrintDialogPage;
class KPrintDialog;
class KPrinterPropertyDialog;

class KMUiManager : public QObject
{
public:
	enum PrintDialogFlagType
	{
		Properties     = 0x0001,
		Default        = 0x0002,
		Preview        = 0x0004,
		OutputToFile   = 0x0008,
		Options        = 0x0010,
		PrintDialogAll = 0x00FF
	};
	enum CopyFlagType
	{
		Current  = 0x01,
		Range    = 0x02,
		Collate  = 0x04,
		Order    = 0x08,
		PageSet  = 0x10,
		CopyAll  = 0xFF
	};

	KMUiManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMUiManager();

	// print management
	virtual void addPropertyPages(KMPropertyPage*);
	virtual void setupWizard(KMWizard*);

	// print dialog
	void addPrintDialogPage(KPrintDialogPage *page)		{ m_printdialogpages.append(page); }
	void addPrintDialogStandardPage(int p)			{ m_printdialogstd |= p; }
	void removePrintDialogStandardPage(int p)		{ m_printdialogstd = (m_printdialogstd & ~p); }
	void setPageSelection(KPrinter::PageSelectionType t)	{ m_pageselection = t; }
	int pageSelection() const 				{ return m_pageselection; }
	void setApplicationType(KPrinter::ApplicationType t);
	int applicationType() const 				{ return m_applicationtype; }
	int copyFlags()						{ return (m_pageselection == KPrinter::SystemSide ? m_copyflags : KMUiManager::CopyAll); }
	void setupPrintDialog(KPrintDialog*);

	// printer property dialog
	void setupPropertyDialog(KPrinterPropertyDialog*);
	virtual void setupPrinterPropertyDialog(KPrinterPropertyDialog*);

protected:
	int			m_printdialogflags;
	int 			m_copyflags;
	int 			m_printdialogstd;
	QList<KPrintDialogPage>	m_printdialogpages;
	KPrinter::PageSelectionType	m_pageselection;
	KPrinter::ApplicationType	m_applicationtype;
};

#endif
