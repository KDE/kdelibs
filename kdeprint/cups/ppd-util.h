#ifndef PPDUTIL_H
#define	PPDUTIL_H

#include <cups/ppd.h>
#include <qstring.h>
#include "optionset.h"

QString ppdConflictErrorMsg(ppd_file_t *ppd);
void optionSetToCupsOptions(const OptionSet& opts, int& num_options, cups_option_t **options);

#endif
