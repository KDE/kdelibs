#ifndef KPRINTER_H
#define KPRINTER_H

#include <qpaintdevice.h>
#include <qprinter.h>
#include <qstringlist.h>

#include <kdeprint/optionset.h>

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
	KPrinter	*printer_;
};

//**************************************************************************************

class KDialogImpl;
class KPrintDialogPage;

class KPrinter : public QPaintDevice
{
friend class KPrinterWrapper;
public:
	enum StandardPageType
	{
		CopiesPage    = 0x01,
		SizeColorPage = 0x02
	};
	enum DialogFlagType
	{
		Properties   = 0x0001,
		Default      = 0x0002,
		Preview      = 0x0004,
		OutputToFile = 0x0008,
		Options      = 0x0010,
		All          = 0x00FF
	};
	enum PageSelectionType
	{
		ApplicationSide = 0x01,
		SystemSide      = 0x02
	};
	enum PageSetType
	{
		AllPages  = 0x00,
		OddPages  = 0x01,
		EvenPages = 0x02
	};
	enum CollateType
	{
		Collate =   0x00,
		Uncollate = 0x01
	};
	enum ApplicationType
	{
		Dialog =     0x00,
		StandAlone = 0x01
	};

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

	KPrinter();
	~KPrinter();

	// QPrinter interface
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
	virtual QSize margins() const;
	bool currentPage() const;
	PageSetType pageSet() const;
	bool setup(QWidget *parent = 0);

	KDialogImpl* implementation() const;
	void addDialogPage(KPrintDialogPage*);

	PageSelectionType pageSelection() const;
	void setPageSelection(PageSelectionType);
	ApplicationType applicationType() const;
	void setApplicationType(ApplicationType);

	void addDialogFlags(int f);
	void removeDialogFlags(int f);
	void addStandardPage(int p);
	void removeStandardPage(int p);

	const QString& option(const QString& key) const;
	void setOption(const QString& key, const QString& value);
	const OptionSet& options() const;
	void setOptions(const OptionSet& opts);
	void initOptions(const OptionSet& opts);

	QString searchName() const;
	void setSearchName(const QString& n);

	virtual bool printFile(const QString& filename);
	virtual bool printFiles(const QStringList& filenames);
	void preparePrinting();
	virtual int copyFlags() const;

protected:
	virtual bool cmd(int, QPainter*, QPDevCmdParam*);
	virtual void translateOptions(const OptionSet& opts);
	virtual int metric(int) const;
	virtual bool supportPrinterOptions() const;

protected:
	KDialogImpl	*impl_;
	KPrinterWrapper	*wrapper_;
	OptionSet	qtoptions_;
	QString		psbuffer_, tmpbuffer_;
	bool		outputtofile_;
	QString		printername_;
	QString		searchname_;
	PageSelectionType	pageselection_;
	ApplicationType	apptype_;
};

inline KDialogImpl* KPrinter::implementation() const
{ return impl_; }

inline const QString& KPrinter::option(const QString& key) const
{ return qtoptions_[key]; }

inline void KPrinter::setOption(const QString& key, const QString& value)
{ qtoptions_[key] = value; }

inline QString KPrinter::docName() const
{ return option("qt-docname"); }

inline void KPrinter::setDocName(const QString& d)
{ setOption("qt-docname",d); }

inline QString KPrinter::creator() const
{ return option("qt-creator"); }

inline void KPrinter::setCreator(const QString& d)
{ setOption("qt-creator",d); }

inline bool KPrinter::fullPage() const
{ return (option("qt-fullpage") == "1"); }

inline void KPrinter::setFullPage(bool on)
{ setOption("qt-fullpage",(on ? "1" : "0")); }

inline KPrinter::ColorMode KPrinter::colorMode() const
{ return (option("qt-colormode") == "GrayScale" ? GrayScale : Color); }

inline void KPrinter::setColorMode(ColorMode m)
{ setOption("qt-colormode",(m == Color ? "Color" : "GrayScale")); }

inline void KPrinter::setNumCopies(int n)
{ setOption("qt-copies",QString::number(n)); }

inline KPrinter::Orientation KPrinter::orientation() const
{ return (option("qt-orientation") == "Landscape" ? Landscape : Portrait); }

inline KPrinter::PageOrder KPrinter::pageOrder() const
{ return (option("qt-pageorder") == "Reverse" ? LastPageFirst : FirstPageFirst); }

inline void KPrinter::setPageOrder(PageOrder o)
{ setOption("qt-pageorder",(o == LastPageFirst ? "Reverse" : "Forward")); }

inline KPrinter::CollateType KPrinter::collate() const
{ return (option("qt-collate") == "Collate" ? Collate : Uncollate); }

inline void KPrinter::setCollate(CollateType c)
{ setOption("qt-collate",(c == Collate ? "Collate" : "Uncollate")); }

inline int KPrinter::minPage() const
{ return (option("qt-minpage").isEmpty() ? 0 : option("qt-minpage").toInt()); }

inline int KPrinter::maxPage() const
{ return (option("qt-maxpage").isEmpty() ? 0 : option("qt-maxpage").toInt()); }

inline void KPrinter::setMinMax(int m, int M)
{ setOption("qt-minpage",QString::number(m)); setOption("qt-maxpage",QString::number(M)); }

inline int KPrinter::fromPage() const
{ return (option("qt-frompage").isEmpty() ? 0 : option("qt-frompage").toInt()); }

inline int KPrinter::toPage() const
{ return (option("qt-topage").isEmpty() ? 0 : option("qt-topage").toInt()); }

inline void KPrinter::setFromTo(int m, int M)
{ setOption("qt-frompage",QString::number(m)); setOption("qt-topage",QString::number(M)); setOption("qt-range",(m>0 && M>0 ? QString("%1-%2").arg(m).arg(M) : QString::fromLatin1(""))); }

inline KPrinter::PageSize KPrinter::pageSize() const
{ return (option("qt-pagsize").isEmpty() ? A4 : (PageSize)option("qt-pagesize").toInt()); }

inline QString KPrinter::printerName() const
{ return printername_; }

inline void KPrinter::setPrinterName(const QString& s)
{ printername_ = s; }

inline QString KPrinter::printProgram() const
{ return QString::fromLatin1(""); }

inline void KPrinter::setPrintProgram(const QString&)
{}

inline QString KPrinter::printerSelectionOption() const
{ return QString::fromLatin1(""); }

inline void KPrinter::setPrinterSelectionOption(const QString&)
{}

inline const OptionSet& KPrinter::options() const
{ return qtoptions_; }

inline QString KPrinter::searchName() const
{ return searchname_; }

inline void KPrinter::setSearchName(const QString& s)
{ searchname_ = s; }

inline void KPrinter::preparePrinting()
{ translateOptions(qtoptions_); qtoptions_.dump();}

inline KPrinter::PageSelectionType KPrinter::pageSelection() const
{ return pageselection_; }

inline void KPrinter::setPageSelection(PageSelectionType t)
{ pageselection_ = t; }

inline KPrinter::PageSetType KPrinter::pageSet() const
{ return (option("qt-pageset").isEmpty() ? AllPages : (PageSetType)(option("qt-pageset").toInt())); }

inline bool KPrinter::currentPage() const
{ return (option("qt-current") == "1"); }

inline KPrinter::ApplicationType KPrinter::applicationType() const
{ return apptype_; }

inline void KPrinter::setApplicationType(ApplicationType t)
{ apptype_ = t; }

//**************************************************************************************

class KJobLister;

class KPrinterFactory
{
public:
	static KPrinterFactory* self();
	static void release();

	enum PrintSystemType { LPD = 0, CUPS, PDQ };

	KPrinterFactory();
	~KPrinterFactory();

	KPrinter* printer();
	KJobLister* jobLister();
	PrintSystemType printSystem() const;
	bool setPrintSystem(PrintSystemType);

private:
	void createPrinter();
	void createLister();
	void loadConfig();

private:
	static KPrinterFactory*	unique_;

	KPrinter*	printer_;
	KJobLister*	joblister_;
	PrintSystemType	type_;
};

inline KPrinterFactory::PrintSystemType KPrinterFactory::printSystem() const
{ return type_; }

//**************************************************************************************

KPrinter::PageSize pageNameToPageSize(const QString& name);
const char* pageSizeToPageName(KPrinter::PageSize s);
QSize rangeToSize(const QString& s);

#endif
