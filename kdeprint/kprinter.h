#ifndef KPRINTER_H
#define KPRINTER_H

#include <qpaintdevice.h>
#include <qprinter.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

class KPrinterWrapper : public QPrinter
{
friend class KPrinter;
public:
	KPrinterWrapper(KPrinter*);
	~KPrinterWrapper();
protected:
	virtual bool cmd(int, QPainter*, QPDevCmdParam*);
	virtual int metric(int) const;
	int qprinterMetric(int) const;
private:
	KPrinter	*m_printer;
};

//**************************************************************************************

class KPrinterImpl;
class KPrintDialogPage;

class KPrinter : public QPaintDevice
{
friend class KPrinterWrapper;
friend class KPrinterImpl;
public:
	// Print global settings (set via static functions)
	enum StandardPageType { CopiesPage = 0x01, Custom = 0x10 };
	enum PageSelectionType { ApplicationSide = 0x01, SystemSide = 0x02 };
	enum ApplicationType { Dialog = 0x00, StandAlone = 0x01 };

	// QPrinter extension
	enum PageSetType { AllPages = 0x00, OddPages = 0x01, EvenPages = 0x02 };
	enum CollateType { Collate = 0x00, Uncollate = 0x01 };

	// from QPrinter class
	enum ColorMode { GrayScale = QPrinter::GrayScale, Color = QPrinter::Color };
	enum Orientation { Portrait = QPrinter::Portrait, Landscape = QPrinter::Landscape };
	enum PageOrder { FirstPageFirst = QPrinter::FirstPageFirst, LastPageFirst = QPrinter::LastPageFirst };
	enum PageSize
	{
		A4 = QPrinter::A4,
		B5 = QPrinter::B5,
		Letter = QPrinter::Letter,
		Legal = QPrinter::Legal,
		Executive = QPrinter::Executive,
		A0 = QPrinter::A0,
		A1 = QPrinter::A1,
		A2 = QPrinter::A2,
		A3 = QPrinter::A3,
		A5 = QPrinter::A5,
		A6 = QPrinter::A6,
		A7 = QPrinter::A7,
		A8 = QPrinter::A8,
		A9 = QPrinter::A9,
		B0 = QPrinter::B0,
		B1 = QPrinter::B1,
		B10 = QPrinter::B10,
		B2 = QPrinter::B2,
		B3 = QPrinter::B3,
		B4 = QPrinter::B4,
		B6 = QPrinter::B6,
		B7 = QPrinter::B7,
		B8 = QPrinter::B8,
		B9 = QPrinter::B9,
		C5E = QPrinter::C5E,
		Comm10E = QPrinter::Comm10E,
		DLE = QPrinter::DLE,
		Folio = QPrinter::Folio,
		Ledger = QPrinter::Ledger,
		Tabloid = QPrinter::Tabloid,
		NPageSize = QPrinter::NPageSize
	};

	// constructors / destructor
	KPrinter();
	~KPrinter();

	// QPrinter interface (+ some extensions)
	bool newPage();
	bool abort();
	bool aborted() const;
	bool outputToFile() const;
	void setOutputToFile(bool);
	QString outputFileName() const;
	void setOutputFileName(const QString&);
	QString docName() const;
	void setDocName(const QString&);
	QString creator() const;
	void setCreator(const QString&);
	bool fullPage() const;
	void setFullPage(bool);
	ColorMode colorMode() const;
	void setColorMode(ColorMode);
	int numCopies() const;
	void setNumCopies(int n);
	Orientation orientation() const;
	void setOrientation(Orientation);
	PageOrder pageOrder() const;
	void setPageOrder(PageOrder);
	CollateType collate() const;
	void setCollate(CollateType);
	int minPage() const;
	int maxPage() const;
	void setMinMax(int, int);
	int fromPage() const;
	int toPage() const;
	void setFromTo(int, int);
	PageSize pageSize() const;
	void setPageSize(PageSize);
	QString printerName() const;
	void setPrinterName(const QString&);
	QString printProgram() const;
	void setPrintProgram(const QString&);
	QString printerSelectionOption() const;
	void setPrinterSelectionOption(const QString&);
	bool currentPage() const;
	PageSetType pageSet() const;
	bool setup(QWidget *parent = 0);
	QSize margins() const;
	void setMargins(QSize m);
	QSize realPageSize() const;
	void setRealPageSize(QSize p);

	KPrinterImpl* implementation() const;
	bool printFiles(const QStringList&);
	void preparePrinting();

	static void addDialogPage(KPrintDialogPage*);
	static void setPageSelection(PageSelectionType);
	static PageSelectionType pageSelection();
	static void setApplicationType(ApplicationType);
	static ApplicationType applicationType();
	static void addStandardPage(int p);
	static void removeStandardPage(int p);

	const QString& option(const QString& key) const;
	void setOption(const QString& key, const QString& value);
	const QMap<QString,QString>& options() const;
	void setOptions(const QMap<QString,QString>& opts);
	void initOptions(const QMap<QString,QString>& opts);

	QString searchName() const;
	void setSearchName(const QString& n);

protected:
	virtual bool cmd(int, QPainter*, QPDevCmdParam*);
	virtual int metric(int) const;
	void translateQtOptions();
	void loadSettings();
	void saveSettings();

protected:
	KPrinterImpl		*m_impl;
	KPrinterWrapper		*m_wrapper;
	QMap<QString,QString>	m_options;
	QString			m_psbuffer, m_tmpbuffer;
	bool			m_outputtofile;
	QString			m_printername;
	QString			m_searchname;
	QSize			m_margins;
	QSize			m_pagesize;
};

inline KPrinterImpl* KPrinter::implementation() const
{ return m_impl; }

inline const QString& KPrinter::option(const QString& key) const
{ return m_options[key]; }

inline void KPrinter::setOption(const QString& key, const QString& value)
{ m_options[key] = value; }

inline QString KPrinter::docName() const
{ return option("kde-docname"); }

inline void KPrinter::setDocName(const QString& d)
{ setOption("kde-docname",d); }

inline QString KPrinter::creator() const
{ return option("kde-creator"); }

inline void KPrinter::setCreator(const QString& d)
{ setOption("kde-creator",d); }

inline bool KPrinter::fullPage() const
{ return (option("kde-fullpage") == "1"); }

inline void KPrinter::setFullPage(bool on)
{ setOption("kde-fullpage",(on ? "1" : "0")); }

inline KPrinter::ColorMode KPrinter::colorMode() const
{ return (option("kde-colormode") == "GrayScale" ? GrayScale : Color); }

inline void KPrinter::setColorMode(ColorMode m)
{ setOption("kde-colormode",(m == Color ? "Color" : "GrayScale")); }

inline void KPrinter::setNumCopies(int n)
{ setOption("kde-copies",QString::number(n)); }

inline KPrinter::Orientation KPrinter::orientation() const
{ return (option("kde-orientation") == "Landscape" ? Landscape : Portrait); }

inline KPrinter::PageOrder KPrinter::pageOrder() const
{ return (option("kde-pageorder") == "Reverse" ? LastPageFirst : FirstPageFirst); }

inline void KPrinter::setPageOrder(PageOrder o)
{ setOption("kde-pageorder",(o == LastPageFirst ? "Reverse" : "Forward")); }

inline KPrinter::CollateType KPrinter::collate() const
{ return (option("kde-collate") == "Collate" ? Collate : Uncollate); }

inline void KPrinter::setCollate(CollateType c)
{ setOption("kde-collate",(c == Collate ? "Collate" : "Uncollate")); }

inline int KPrinter::minPage() const
{ return (option("kde-minpage").isEmpty() ? 0 : option("kde-minpage").toInt()); }

inline int KPrinter::maxPage() const
{ return (option("kde-maxpage").isEmpty() ? 0 : option("kde-maxpage").toInt()); }

inline void KPrinter::setMinMax(int m, int M)
{ setOption("kde-minpage",QString::number(m)); setOption("kde-maxpage",QString::number(M)); }

inline int KPrinter::fromPage() const
{ return (option("kde-frompage").isEmpty() ? 0 : option("kde-frompage").toInt()); }

inline int KPrinter::toPage() const
{ return (option("kde-topage").isEmpty() ? 0 : option("kde-topage").toInt()); }

inline void KPrinter::setFromTo(int m, int M)
{ setOption("kde-frompage",QString::number(m)); setOption("kde-topage",QString::number(M)); setOption("kde-range",(m>0 && M>0 ? QString("%1-%2").arg(m).arg(M) : QString::fromLatin1(""))); }

inline KPrinter::PageSize KPrinter::pageSize() const
{ return (option("kde-pagesize").isEmpty() ? A4 : (PageSize)option("kde-pagesize").toInt()); }

inline KPrinter::PageSetType KPrinter::pageSet() const
{ return (option("kde-pageset").isEmpty() ? AllPages : (PageSetType)(option("kde-pageset").toInt())); }

inline bool KPrinter::currentPage() const
{ return (option("kde-current") == "1"); }

inline QString KPrinter::printerName() const
{ return m_printername; }

inline void KPrinter::setPrinterName(const QString& s)
{ m_printername = s; }

inline QString KPrinter::printProgram() const
{ return QString::fromLatin1(""); }

inline void KPrinter::setPrintProgram(const QString&)
{}

inline QString KPrinter::printerSelectionOption() const
{ return QString::fromLatin1(""); }

inline void KPrinter::setPrinterSelectionOption(const QString&)
{}

inline const QMap<QString,QString>& KPrinter::options() const
{ return m_options; }

inline QString KPrinter::searchName() const
{ return m_searchname; }

inline void KPrinter::setSearchName(const QString& s)
{ m_searchname = s; }

inline bool KPrinter::newPage()
{ return m_wrapper->newPage(); }

inline QString KPrinter::outputFileName() const
{ return m_psbuffer; }

inline bool KPrinter::outputToFile() const
{ return m_outputtofile; }

inline bool KPrinter::abort()
{ return m_wrapper->abort(); }

inline bool KPrinter::aborted() const
{ return m_wrapper->aborted(); }

inline void KPrinter::setMargins(QSize m)
{ m_margins = m; }

inline QSize KPrinter::realPageSize() const
{ return m_pagesize; }

inline void KPrinter::setRealPageSize(QSize p)
{ m_pagesize = p; }

//**************************************************************************************

// Utility functions

KPrinter::PageSize pageNameToPageSize(const QString& name);
const char* pageSizeToPageName(KPrinter::PageSize s);
QSize rangeToSize(const QString& s);

#endif
