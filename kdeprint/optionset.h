#ifndef OPTIONSET_H
#define OPTIONSET_H

#include <qmap.h>
#include <qstring.h>

typedef QMap<QString,QString>	OptionSetBase;

class OptionSet : public OptionSetBase
{
public:
	const QString& option(const QString& key);
	void setOption(const QString& key, const QString& value);
	void merge(const OptionSet& o);

	void dump() const;
};

#endif
