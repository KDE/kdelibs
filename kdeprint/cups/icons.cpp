#include "icons.h"

#include <cups/language.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void openLang();
void closeLang();
static cups_lang_t	*lang_ = 0;

bool isGeneralGroup(const char *txt);
bool isInstallGroup(const char *txt);
bool isJCLGroup(const char *txt);
bool isExtraGroup(const char *txt);

//-------------------------------------------------------------------------

void openLang()
{
	if (!lang_) lang_ = cupsLangDefault();
	if (!lang_) exit(-1);
}

void closeLang()
{
	if (lang_) cupsLangFree(lang_);
}

//-------------------------------------------------------------------------

bool isGeneralGroup(const char *txt)
{
	openLang();
	return (strcmp(txt,cupsLangString(lang_,CUPS_MSG_GENERAL)) == 0);
}

bool isExtraGroup(const char *txt)
{
	openLang();
	return (strcmp(txt,cupsLangString(lang_,CUPS_MSG_EXTRA)) == 0);
}

bool isJCLGroup(const char *txt)
{
	return (strcmp(txt,"JCL") == 0);
}

bool isInstallGroup(const char *txt)
{
	char	*str = strdup(txt);
	for (char *c = str;*c;c++) *c = tolower(*c);
	bool	found = (strstr(str,"install") != NULL);
	free(str);
	return found;
}

//-------------------------------------------------------------------------

static const char*	group_db[] = {
	"kdeprint_group_general",
	"kdeprint_group_extra",
	"kdeprint_group_jcl",
	"kdeprint_group_install"
};

const char* getGroupIconName(const char *txt)
{
	int	index(-1);
	if (isGeneralGroup(txt)) index = 0;
	else if (isExtraGroup(txt)) index = 1;
	else if (isJCLGroup(txt)) index = 2;
	else if (isInstallGroup(txt)) index = 3;
	closeLang();
	return (index >= 0 ? group_db[index] : NULL);
}

//-------------------------------------------------------------------------

typedef struct
{
	const char	*name;
	const char	*iconname;
} icon_entry;

static icon_entry	icon_db[] = {
	{ "PageSize", "kdeprint_pagesize" },
	{ "MediaType", "kdeprint_pagesize" },
	{ "InputSlot", "kdeprint_opt_inputslot" },
	{ "Brightness", "kdeprint_opt_brightness" },
	{ "StpBrightness", "kdeprint_opt_brightness" },
	{ "ColorModel", "kdeprint_opt_color" },
	{ "Grayscale", "kdeprint_opt_color" },
	{ "Yellow", "kdeprint_opt_yellow" },
	{ "StpYellow", "kdeprint_opt_yellow" },
	{ "Cyan", "kdeprint_opt_cyan" },
	{ "StpCyan", "kdeprint_opt_cyan" },
	{ "Magenta", "kdeprint_opt_magenta" },
	{ "StpMagenta", "kdeprint_opt_magenta" },
	{ "Gamma", "kdeprint_opt_gamma" },
	{ "StpGamma", "kdeprint_opt_gamma" },
	{ "Duplex", "kdeprint_opt_duplex" },
	{ "GSResolution", "kdeprint_opt_gsresol" },
	{ "Resolution", "kdeprint_opt_resol" },
	{ "Saturation", "kdeprint_opt_saturation" },
	{ "StpSaturation", "kdeprint_opt_saturation" },
	{ "Contrast", "kdeprint_opt_contrast" },
	{ "StpContrast", "kdeprint_opt_contrast" },
	{ "Dither", "kdeprint_opt_dither" },
	{ "Dithering", "kdeprint_opt_dither" },
	{ "ImageType", "kdeprint_opt_imagetype" },
	{ "Quality", "kdeprint_pagesize" }
};

const char* getOptionIconName(const char *optionname)
{
	int	db_size = sizeof(icon_db)/sizeof(icon_entry);
	for (int i=0;i<db_size;i++)
		if (strcasecmp(optionname,icon_db[i].name) == 0) return icon_db[i].iconname;
	return NULL;
}
