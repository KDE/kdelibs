
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
#include "kimgio.h"
#include <klocale.h>

#ifdef LINKED_png
extern "C" void kimgio_init_png();
#endif

#ifdef LINKED_jpeg
extern "C" void kimgio_init_jpeg();
#endif

#ifdef LINKED_tiff
extern "C" void kimgio_init_tiff();
#endif

extern "C" void kimgio_init_krl();

#ifdef HAVE_QIMGIO
#include <qimageio.h>
#endif

void kimgioRegister(void)
{
	if( registered ) {
	    return;
	}

	registered = 1;

	lt_dlinit();

	// for the libraries that are installed in $KDEDIR/lib (kdesupport)
	lt_dladdsearchdir((KApplication::kde_bindir() + "/../lib").ascii());
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
	    lt_dlhandle libhandle = lt_dlopen(libname.ascii());
	    if (libhandle == 0) {
		warning("couldn't dlopen %s (%s)",
			libname.ascii(), lt_dlerror());
	    }
	
	    libname.replace(QRegExp("^.*kimg_"), "kimgio_init_");
	    libname.replace(QRegExp("\\.la$"), "");
	
	    lt_ptr_t init_func = lt_dlsym(libhandle, libname.ascii() );
	
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

#ifdef HAVE_QIMGIO
        qInitImageIO();
#endif

}

QString kimgio_patterns() {

  // this is hardcoded for the moment - but better hardcoded centrally :/
  return i18n( "*.gif *GIF *.bmp *.BMP *.xbm *.XBM *.xpm *.XPM *.pnm *.PNM "
	       "*.PBM *.PGM *.PPM *.PBMRAW *.PGMRAW *.PPMRAW *.jpg *.JPG *.jpeg *.JPEG "
	       "*.pbm *.pgm *.ppm *.pbmdraw *.pgmdraw *.ppmdraw *.png *.PNG|All pictures\n"
	       "*.png *.PNG|PNG-Pictures\n"
	       "*.gif *.GIF|GIF-Pictures\n"
	       "*.jpg *.JPG *.jpeg *.JPEG|JPEG-Pictures\n"
	       "*.bmp *.BMP|Windows Bitmaps\n"
	       "*.xbm *.XBM|XWindow Pitmaps\n"
	       "*.xpm *.XPM|Pixmaps\n"
	       "*.pnm *.PNM *.PBM *.PGM *.PPM *.PBMRAW *.PGMRAW *.PPMRAW "
	       "*.pbm *.pgm *.ppm *.pbmdraw *.pgmdraw *.ppmdraw|PNM-Pictures" );
}

bool KImageIO::canWrite(const QString& type)
{
#ifdef HAVE_QIMGIO
  if (type == "JPEG")
    return true;
#endif
  if (type == "XPM" || type == "XBM" || type == "PNG" || type == "BMP")
    return true;

  return false;
}

bool KImageIO::canRead(const QString& type)
{
  if (type == "JPEG" || 
      type == "XPM" || type == "XBM" || type == "PNG" || type == "BMP")
    return true;
  
  return false;
}

QStringList kimgio_types() {
  static QStringList types;
  if (types.isEmpty()) {
    types.append("GIF");
#ifdef HAVE_QIMGIO
    types.append("JPEG");
#endif
    types.append("JPG");
    types.append("XPM");
    types.append("XBM");
    types.append("PNG");
  }
  return types;
}
