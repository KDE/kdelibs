#include "kmdbentry.h"

bool KMDBEntry::validate()
{
	// check model
	if (model.isEmpty())
	{
		model = modelname;
		if (model.isEmpty()) return false;
	}

	// check manufacturer
	if (manufacturer.isEmpty())
	{
		int	p = model.find(' ',0);
		if (p != -1) manufacturer = model.left(p);
		if (manufacturer.isEmpty()) return false;
	}

	// extract manufacturer from model
	if (model.find(manufacturer,0,false) == 0)
	{
		model = model.right(model.length()-manufacturer.length()-1).stripWhiteSpace();
		if (model.isEmpty()) return false;
	}

	return true;
}
