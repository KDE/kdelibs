 
/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

static int registered = 0;

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#include <qdir.h>
#include <kapp.h>
#include <qstring.h>
#include <qregexp.h>

#include <ltdl.h>
#include "kimgio-config.h"

#ifdef LINKED_png
extern "C" void kimgio_init_png();
#endif

#ifdef LINKED_jpeg
extern "C" void kimgio_init_jpeg();
#endif

#ifdef LINKED_tiff
extern "C" void kimgio_init_tiff();
#endif

void kimgioRegister(void)
{
	if( registered ) {
	    return;
	}

	registered = 1;

	lt_dlinit();

	// for the libraries that are installed in $KDEDIR/lib (kdesupport)
	lt_dladdsearchdir(KApplication::kde_bindir() + "/../lib");
/*
  Disabled by David, to fix compilation. No idea where this is defined - Stephan ?
	int rindex = 0;
	while (kimgio_rpaths[rindex] != 0)
	    lt_dladdsearchdir(kimgio_rpaths[rindex++]);
*/

	QDir dir(KApplication::kde_bindir() + "/../lib", "kimg_*.la");

	for (uint index = 0; index < dir.count(); index++) {
	    
	    QString libname = dir[index];
	    if (libname.isNull())
		break;

	    libname = dir.path() + "/" + libname;
	    lt_dlhandle libhandle = lt_dlopen(libname);
	    if (libhandle == 0) {
		warning("couldn't dlopen %s (%s)", 
			libname.ascii(), lt_dlerror());
	    }
	    
	    libname.replace(QRegExp("^.*kimg_"), "kimgio_init_");
	    libname.replace(QRegExp("\\.la$"), "");
	    
	    lt_ptr_t init_func = lt_dlsym(libhandle, libname );
	
	    if (init_func == NULL) {
		warning("couldn't init module %s (%s)!", libname.ascii(), 
			lt_dlerror());
	    } else {
		void (*func)();
		func = (void (*)(void))init_func;
		func();
	    }
	}

#ifdef LINKED_png
	kimgio_init_png();
#endif

#ifdef LINKED_jpeg
	kimgio_init_jpeg();
#endif

#ifdef LINKED_tiff
	kimgio_init_tiff();
#endif
      
}
