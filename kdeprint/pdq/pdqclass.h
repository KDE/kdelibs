#ifndef PDQCLASS_H
#define PDQCLASS_H

#include "pdq.h"

#include <qlist.h>
#include <qdict.h>
#include <qmap.h>
#include <qtextstream.h>

class PDQMain;
class PDQPrinter;
class PDQDriver;
class PDQInterface;
class PDQArgument;
class PDQOption;
class PDQOption;
class PDQChoice;
class PDQFilter;

class PDQBase
{
public:
	PDQBase();
	PDQBase(const PDQBase& bs);
	virtual ~PDQBase();

	bool readBlock(BlockT *blk);
	virtual bool processString(const QString& name, const QString& value);
	virtual bool processStringList(const QString& name, const QStringList& values);
	virtual bool processPairList(const QString& name, const QMap<QString,QString>& pairs);
	virtual bool processBlock(const QString& name, BlockT *blk);
	virtual void writeToFile(QTextStream& t);
	QString name() const { return name_; }
	void setName(const QString& name) { name_ = name; }

public:
	QString	name_;
};

//*************************************************************************************************************

class PDQMain : public PDQBase
{
public:
	PDQMain();
	~PDQMain();

	bool processString(const QString& name, const QString& value);
	bool processBlock(const QString& name, BlockT *blk);
	void writeToFile(QTextStream& t);

	void init();
	bool parseFile(const QString& filename, bool expandincl = true);
	bool saveFile(const QString& filename);
	PDQPrinter* printer(const QString& name);
	PDQDriver* driver(const QString& name);
	PDQInterface* interface(const QString& name);
	void setPrinter(PDQPrinter *pr);
	void setDefaultPrinter(const QString& prname);

public:
	QDict<PDQPrinter>	printers_;
	QDict<PDQInterface>	interfaces_;
	QDict<PDQDriver>	drivers_;
	QString		defprinter_;
	QString		interface_cmd_path_, driver_cmd_path_;
	QStringList	tryinclude_;
	QStringList	include_;
	QString		jobdir_;
};

inline PDQPrinter* PDQMain::printer(const QString& name)
{ return printers_.find(name); }

inline PDQDriver* PDQMain::driver(const QString& name)
{ return drivers_.find(name); }

inline PDQInterface* PDQMain::interface(const QString& name)
{ return interfaces_.find(name); }

inline void PDQMain::setDefaultPrinter(const QString& prname)
{ defprinter_ = prname; }

//*************************************************************************************************************

class PDQPrinter : public PDQBase
{
public:
	PDQPrinter();
	PDQPrinter(const PDQPrinter& pr);
	~PDQPrinter();

	PDQPrinter* duplicate();
	bool processString(const QString& name, const QString& value);
	bool processStringList(const QString& name, const QStringList& values);
	bool processPairList(const QString& name, const QMap<QString,QString>& pairs);
	void writeToFile(QTextStream& t);

public:
	QString	interface_;
	QString	driver_;
	QString	model_;
	QString	location_;
	QStringList	driver_options_;
	QStringList	interface_options_;
	QMap<QString,QString>	driver_args_;
	QMap<QString,QString>	interface_args_;
};

//*************************************************************************************************************

class PDQInterface : public PDQBase
{
public:
	PDQInterface();
	~PDQInterface();

	bool processString(const QString& name, const QString& value);
	bool processStringList(const QString& name, const QStringList& values);
	bool processBlock(const QString& name, BlockT *blk);

public:
	QString	verify_;
	QString	send_;
	QString	status_;
	QString	cancel_;
	QString	help_;
	QStringList	required_args_;
	QStringList	requires_;
	QList<PDQArgument>	arguments_;
	QList<PDQOption>	options_;
};

//*************************************************************************************************************

class PDQDriver : public PDQBase
{
public:
	PDQDriver();
	~PDQDriver();

	bool processString(const QString& name, const QString& value);
	bool processStringList(const QString& name, const QStringList& values);
	bool processBlock(const QString& name, BlockT *blk);

public:
	QString	verify_;
	QString	filetype_;
	QString	filter_;
	QString	help_;
	QStringList	required_args_;
	QStringList	requires_;
	QList<PDQArgument>	arguments_;
	QList<PDQOption>	options_;
	QList<PDQFilter>	language_drivers_;
};

//*************************************************************************************************************

class PDQArgument : public PDQBase
{
public:
	PDQArgument();
	~PDQArgument();

	bool processString(const QString& name, const QString& value);

public:
	QString	defvalue_;
	QString	description_;
	QString	help_;
};

//*************************************************************************************************************

class PDQChoice : public PDQBase
{
public:
	PDQChoice();
	~PDQChoice();

	bool processString(const QString& name, const QString& value);

public:
	QString	value_;
	QString	description_;
	QString	help_;
};

//*************************************************************************************************************

class PDQOption : public PDQBase
{
public:
	PDQOption();
	~PDQOption();

	bool processString(const QString& name, const QString& value);
	bool processBlock(const QString& name, BlockT *blk);

public:
	QString	default_choice_;
	QString	description_;
	QList<PDQChoice>	choices_;
};

//*************************************************************************************************************

class PDQFilter : public PDQBase
{
public:
	PDQFilter();
	~PDQFilter();

	bool processString(const QString& name, const QString& value);

public:
	QString	filetype_regx_;
	QString	convert_;
};

#endif
