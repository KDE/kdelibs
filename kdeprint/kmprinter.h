/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KMPRINTER_H
#define KMPRINTER_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <qstring.h>
#include <qstringlist.h>
#include <q3ptrlist.h>
#include <kurl.h>
#include <qmap.h>
#include <kdeprint/kmobject.h>

class KMDBEntry;
class DrMain;
class KPrinter;
class QWidget;

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT KMPrinter : public KMObject
{
public:
	enum PrinterType {
		Printer	 = 0x01,
		Class    = 0x02,
		Implicit = 0x04,
		Virtual  = 0x08,
		Remote   = 0x10,
		Invalid  = 0x20,
		Special  = 0x40
	};
	enum PrinterState {
		Idle       = 0x01,
		Stopped    = 0x02,
		Processing = 0x03,
		Unknown    = 0x04,
		Rejecting  = 0x08,
		StateMask  = 0x07
	};
	enum PrinterCap {
		CapBW       = 0x0001,
		CapColor    = 0x0002,
		CapDuplex   = 0x0004,
		CapStaple   = 0x0008,
		CapCopies   = 0x0010,
		CapCollate  = 0x0020,
		CapPunch    = 0x0040,
		CapCover    = 0x0080,
		CapBind     = 0x0100,
		CapSort     = 0x0200,
		CapSmall    = 0x0400,
		CapMedium   = 0x0800,
		CapLarge    = 0x1000,
		CapVariable = 0x2000
	};

	// constructor functions
	KMPrinter();
	KMPrinter(const KMPrinter& p);
	~KMPrinter();
	void copy(const KMPrinter& p);


	// inline access functions
	const QString& name() const		{ return m_name; }
	void setName(const QString& s)		{ m_name = s; if (m_printername.isEmpty()) m_printername = s; }
	const QString& printerName() const	{ return m_printername; }
	void setPrinterName(const QString& s)	{ m_printername = s; if (m_name.isEmpty()) m_name = s; }
	const QString& instanceName() const	{ return m_instancename; }
	void setInstanceName(const QString& s)	{ m_instancename = s; }
	int type() const 			{ return m_type; }
	void setType(int t) 			{ m_type = t; }
	void addType(int t) 			{ m_type |= t; }
	PrinterState state(bool complete = false) const 		{ return PrinterState(m_state & (complete ? ~0x0 : StateMask)); }
	QString stateString() const;
	void setState(PrinterState s) 		{ m_state = PrinterState((m_state & ~StateMask) | s); }
	bool acceptJobs() const			{ return !(m_state & Rejecting); }
	void setAcceptJobs(bool on)		{ m_state = PrinterState((m_state & StateMask) | (on ? 0 : Rejecting)); }
	const QString& device() const 		{ return m_device; }
	QString deviceProtocol() const;
	void setDevice(const QString& d) 		{ m_device = d; }
	const QStringList& members() const	{ return m_members; }
	void setMembers(const QStringList& l)	{ m_members = l; }
	void addMember(const QString& s)	{ m_members.append(s); }
	const QString& description() const 	{ return m_description; }
	void setDescription(const QString& s) 	{ m_description = s; }
	const QString& location() const 	{ return m_location; }
	void setLocation(const QString& s) 	{ m_location = s; }
	const KUrl& uri() const 		{ return m_uri; }
	void setUri(const KUrl& u) 		{ m_uri = u; }
	const QString& manufacturer() const	{ return m_manufacturer; }
	void setManufacturer(const QString&s )	{ m_manufacturer = s; }
	const QString& model() const		{ return m_model; }
	void setModel(const QString& s)		{ m_model = s; }
	const QString& driverInfo() const	{ return m_driverinfo; }
	void setDriverInfo(const QString& s)	{ m_driverinfo = s; }
	int printerCap() const              { return m_printercap; }
	void setPrinterCap( int c )         { m_printercap = c; }
	void addPrinterCap( int c )         { m_printercap |= c; }

	// some useful functions
	bool isClass(bool useImplicit = true) const	{ return ((m_type & KMPrinter::Class) || (useImplicit && isImplicit())); }
	bool isImplicit() const				{ return (m_type & KMPrinter::Implicit); }
	bool isPrinter() const				{ return (m_type & KMPrinter::Printer); }
	bool isRemote() const				{ return (m_type & KMPrinter::Remote); }
	bool isLocal() const				{ return !isRemote(); }
	bool isVirtual() const				{ return (m_type & KMPrinter::Virtual); }
	bool isValid() const				{ return !(m_type & KMPrinter::Invalid); }
	bool isSpecial() const				{ return (m_type & KMPrinter::Special); }

	// misc
	KMDBEntry* dbEntry() const	{ return m_dbentry; }
	void setDbEntry(KMDBEntry *e)	{ m_dbentry = e; }
	QString pixmap();
	void setPixmap(const QString& s)	{ m_pixmap = s; }
	bool isHardDefault() const	{ return m_harddefault; }
	void setHardDefault(bool on)	{ m_harddefault = on; }
	bool isSoftDefault() const	{ return m_softdefault; }
	void setSoftDefault(bool on)	{ m_softdefault = on; }
	bool ownSoftDefault() const	{ return m_ownsoftdefault; }
	void setOwnSoftDefault(bool on)	{ m_ownsoftdefault = on; }
	static int compare(KMPrinter *p1, KMPrinter *p2);
	QString option(const QString& key) const 		{ return m_options[key]; }
	bool hasOption(const QString& key) const 			{ return m_options.contains(key); }
	void setOption(const QString& key, const QString& value)	{ if (!key.isEmpty()) m_options[key] = value; }
	void removeOption(const QString& key) 				{ m_options.remove(key); }
	QMap<QString,QString> options() const 				{ return m_options; }
	void setOptions(const QMap<QString,QString>& opts)		{ m_options = opts; }
	DrMain* driver() const 						{ return m_driver; }
	void setDriver(DrMain*);
	DrMain* takeDriver();

	// configure an existing KPrinter object
	bool autoConfigure(KPrinter *prt, QWidget *parent = 0);

	// default options
	QString defaultOption(const QString& key) const 		{ return m_defaultoptions[key]; }
	void setDefaultOption(const QString& key, const QString& value)	{ if (!key.isEmpty()) m_defaultoptions[key] = value; }
	QMap<QString,QString> defaultOptions() const 			{ return m_defaultoptions; }
	void setDefaultOptions(const QMap<QString,QString>& opts)	{ m_defaultoptions = opts; }
	// edited options
	QString editedOption(const QString& key) const 		{ return m_editedoptions[key]; }
	void setEditedOption(const QString& key, const QString& value)	{ if (!key.isEmpty()) m_editedoptions[key] = value; }
	QMap<QString,QString> editedOptions() const 			{ return m_editedoptions; }
	void setEditedOptions(const QMap<QString,QString>& opts)	{ m_editedoptions = opts; }
	// edited flag
	bool isEdited() const 	{ return m_isedited; }
	void setEdited(bool on)	{ m_isedited = on; }


protected:
	// mandantory information
	QString		m_name;		// identification name
	QString		m_printername;	// real printer name
	QString		m_instancename;	// instance name (human-readable)
	int		m_type;		// printer type (any PrinterType flag OR-ed together)
	PrinterState	m_state;	// printer state
	/**
	 * Represent the device as a string, to provide native
	 * support for exotic devices. Conversion to URL is done
	 * only when really needed
	 */
	QString		m_device;	// printer device

	// class specific information
	QStringList	m_members;	// members of the class

	// other useful information that should be completed by manager on demand
	QString		m_description;	// short description, comment
	QString		m_location;	// printer location
	KUrl		m_uri;		// URI printer identification
	QString		m_manufacturer;	// printer manufacturer (driver)
	QString		m_model;	// printer model (driver)
	QString		m_driverinfo;	// short driver info (ex: nick name in PPD)

	// DB driver entry (used when creating a printer). Internal use only !!!
	KMDBEntry	*m_dbentry;
	DrMain		*m_driver;
	QString		m_pixmap;

	// default flags
	bool		m_harddefault;
	bool		m_softdefault;
        bool            m_ownsoftdefault;

	// other options (for any use)
	QMap<QString,QString>	m_options;

	// options used for instances, and print library. These options are not
	// overwritten when copying from another KMPrinter object. This allows to
	// to keep the user settings through an application session. The difference
	// between "default" and "edited" can be used for option saving (only save
	// the options corresponding to current edited printer).
	QMap<QString,QString>	m_defaultoptions;
	QMap<QString,QString>	m_editedoptions;
	bool			m_isedited;

	// printer capabilities (based on CUPS model)
	int m_printercap;
};

class KMPrinterList : public Q3PtrList<KMPrinter>
{
public:
	KMPrinterList() : Q3PtrList<KMPrinter>() {}
	virtual int compareItems(Q3PtrCollection::Item i1, Q3PtrCollection::Item i2)
	{ return KMPrinter::compare((KMPrinter*)i1, (KMPrinter*)i2); }
};

#endif
