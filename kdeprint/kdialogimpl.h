#ifndef KDIALOGIMPL_H
#define KDIALOGIMPL_H

#include <qlist.h>
#include <qobject.h>

#include "kprinteritem.h"

class KPrintDialog;
class KPrintDialogPage;
class KPrinterPropertyDialog;
class KPrinter;

class KDialogImpl : public QObject
{
public:
	enum CopyType
	{
		Current = 0x01,
		Range   = 0x02,
		Collate = 0x04,
		Reverse = 0x08,
		PageSet = 0x10,
		Normal  = Range|Collate|Reverse|PageSet
	};

	KDialogImpl(KPrinter *pr, QObject *parent = 0, const char *name = 0);
	~KDialogImpl();

	virtual KPrinterItemList* printersList() = 0;	// called on dialog building to retrieve printers list
	virtual void printerSelected(KPrintDialog *dlg, int index) = 0;	// called when a printer is selected (->update some infos on the dialog)
	virtual KPrinterPropertyDialog* buildPropertyDialog(KPrintDialog *dlg, int index) = 0;	// called when click on "Properties"
	virtual bool checkPrinter(int, QString&);
	virtual void setDefaultPrinter(KPrintDialog *dlg, int index);	// called when click on "Set as default"
	virtual bool configure(KPrintDialog *dlg);

	void addDialogPage(KPrintDialogPage *page);
	QList<KPrintDialogPage>* dialogPages();
	void setDialogFlags(int f);
	int dialogFlags() const;
	void setStandardPages(int p);
	int standardPages() const;

	KPrinterItem* findPrinter(const QString& name);
	KPrinterItem* filePrinter() const;
	void setGlobalOption(const QString& key, const QString& value);

protected:
	void addPrinter(KPrinterItem *printer);
	void setDefault(KPrinterItem *printer);

protected:
	KPrinterItemList	printers_;
	QList<KPrintDialogPage>	pages_;
	int	dialogflags_;
	int	standardpages_;
	int	copyflags_;
	KPrinterItem		*fileprinter_;
	KPrinter		*printer_;
};

inline void KDialogImpl::setDialogFlags(int f)
{ dialogflags_ = f; }

inline int KDialogImpl::dialogFlags() const
{ return dialogflags_; }

inline void KDialogImpl::setStandardPages(int p)
{ standardpages_ = p; }

inline int KDialogImpl::standardPages() const
{ return standardpages_; }

inline KPrinterItem* KDialogImpl::filePrinter() const
{ return fileprinter_; }

#endif
