#ifndef KPRINTERITEM_H
#define KPRINTERITEM_H

#include <qstring.h>
#include <qlist.h>
#include <qpixmap.h>

#include "optionset.h"

class KPrinterItem
{
public:
	enum PrinterType
	{
		Printer   = 0x0001,
		Class     = 0x0002,
		Implicit  = 0x0004,
		Local     = 0x0008,
		Remote    = 0x0010,
		Valid     = 0x0020
	};
	enum PrinterState
	{
		Idle = 0,
		Processing,
		Stopped
	};

	KPrinterItem();
	void copy(const KPrinterItem& printer);
	void clear();

	const QString& displayName() const;
	void setDisplayName(const QString& s);
	const QString& printerName() const;
	void setPrinterName(const QString& s);
	const QString& name() const;
	void setName(const QString& s);
	int type() const;
	void setType(int t);
	void addType(int t);
	const QPixmap& pixmap() const;
	void setPixmap(const QPixmap& pix);
	bool isDefault() const;
	void setDefault(bool on);
	PrinterState state() const;
	void setState(PrinterState);

	bool isDiscarded() const;
	void setDiscarded(bool on);
	bool isEdited() const;
	void setEdited(bool on);

	void addDefaultOption(const QString& name, const QString& value);
	const OptionSet& defaultOptions() const;
	void setDefaultOptions(const OptionSet& opt);
	void addEditedOption(const QString& name, const QString& value);
	const OptionSet& editedOptions() const;
	void setEditedOptions(const OptionSet& opt);

private:
	QString		displayname_;
	QString		printername_;
	QString		name_;
	int		type_;
	QPixmap		pixmap_;
	bool		default_;
	PrinterState	state_;

	OptionSet	defaultset_;
	OptionSet	editedset_;

	bool		discarded_;
	bool		edited_;
};

inline const QString& KPrinterItem::displayName() const
{ return displayname_; }

inline void KPrinterItem::setDisplayName(const QString& s)
{ displayname_ = s; }

inline const QString& KPrinterItem::printerName() const
{ return printername_; }

inline void KPrinterItem::setPrinterName(const QString& s)
{ printername_ = s; }

inline const QString& KPrinterItem::name() const
{ return name_; }

inline void KPrinterItem::setName(const QString& s)
{ name_ = s; }

inline bool KPrinterItem::isDiscarded() const
{ return discarded_; }

inline void KPrinterItem::setDiscarded(bool on)
{ discarded_ = on; }

inline int KPrinterItem::type() const
{ return type_; }

inline void KPrinterItem::setType(int t)
{ type_ = t; }

inline void KPrinterItem::addType(int t)
{ type_ |= t; }

inline const QPixmap& KPrinterItem::pixmap() const
{ return pixmap_; }

inline void KPrinterItem::setPixmap(const QPixmap& pix)
{ pixmap_ = pix; }

inline bool KPrinterItem::isDefault() const
{ return default_; }

inline void KPrinterItem::setDefault(bool on)
{ default_ = on; }

inline bool KPrinterItem::isEdited() const
{ return edited_; }

inline void KPrinterItem::setEdited(bool on)
{ edited_ = on; }

inline void KPrinterItem::addDefaultOption(const QString& name, const QString& value)
{ defaultset_.insert(name,value); }

inline void KPrinterItem::addEditedOption(const QString& name, const QString& value)
{ editedset_.insert(name,value); }

inline const OptionSet& KPrinterItem::defaultOptions() const
{ return defaultset_; }

inline void KPrinterItem::setDefaultOptions(const OptionSet& opt)
{ defaultset_ = opt; }

inline const OptionSet& KPrinterItem::editedOptions() const
{ return editedset_; }

inline void KPrinterItem::setEditedOptions(const OptionSet& opt)
{ editedset_ = opt; }

inline KPrinterItem::PrinterState KPrinterItem::state() const
{ return state_; }

inline void KPrinterItem::setState(PrinterState s)
{ state_ = s; }

//**************************************************************************************

typedef QList<KPrinterItem>	KPrinterItemListBase;
typedef QListIterator<KPrinterItem>	KPrinterItemListIterator;

class KPrinterItemList : public KPrinterItemListBase
{
public:
	KPrinterItemList() : KPrinterItemListBase() {}
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2)
	{
		return QString::compare(((KPrinterItem*)i1)->name(),((KPrinterItem*)i2)->name());
	}
};

#endif
