#include "ksgistyle.h"
#include <klocale.h>

#define		MAJOR_VERSION	1
#define		MINOR_VERSION	0

extern "C" {
	KStyle*			allocate();
	int					minor_version();
	int					major_version();
	const char*	description();
}

KStyle* allocate()
{
	return (new KSgiStyle);
}

int minor_version()
{
	return (MINOR_VERSION);
}

int major_version()
{
	return (MAJOR_VERSION);
}

const char* description()
{
	return (i18n("Sgi Style").utf8());
}
