#include "optionset.h"

const QString& OptionSet::option(const QString& opt)
{
	return (*this)[opt];
}

void OptionSet::setOption(const QString& opt, const QString& val)
{
	insert(opt, val);
}

void OptionSet::dump() const
{
	for (QMap<QString,QString>::ConstIterator it=begin(); it != end(); ++it)
		debug("%s = %s",it.key().latin1(),it.data().latin1());
}

void OptionSet::merge(const OptionSet& o)
{
	for (QMap<QString,QString>::ConstIterator it=o.begin(); it != o.end(); ++it)
		(*this)[it.key()] = it.data();
}
