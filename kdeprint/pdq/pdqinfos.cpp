#include "pdqinfos.h"

#include <kconfig.h>

PdqInfos* PdqInfos::unique_ = 0;

PdqInfos* PdqInfos::self()
{
	if (!unique_)
	{
		unique_ = new PdqInfos();
	}
	return unique_;
}

PdqInfos::PdqInfos()
{
	load();
}

PdqInfos::~PdqInfos()
{
}

void PdqInfos::load()
{
	KConfig	config_("kdeprintrc");
	config_.setGroup("PDQ");
	rcfile_ = config_.readEntry("RCFile","/etc/pdq/printrc");
}

void PdqInfos::save()
{
	KConfig	config_("kdeprintrc");
	config_.setGroup("PDQ");
	config_.writeEntry("RCFile",rcfile_);
}
