/*
 * $Id$
 * Revision 1.1.1.3  1997/12/11 07:19:13  jacek
 * $Log$
 * Various widgets moved from apps to libs
 * Added KSeparator
 * Several bugs fixed
 * Patches from Matthias Ettrich
 * Made ksock.cpp more alpha-friendly
 * Removed XPM-Loading from KPixmap
 * Reaping zombie KDEHelp childs
 * WidgetStyle of KApplication objects configurable via kdisplay
 *
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
 * Sources imported
 *
 * Revision 1.4  1997/03/09 17:43:00  kalle
 * load() returns bool
 *
 * Revision 1.3  1996/12/07 22:12:15  kalle
 * autoconf and documentation
 *
 * Revision 1.2  1996/12/07 22:08:40  kalle
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
#error Declare functions needed from stdlib.h
#endif

 * RCS header
#include <qfile.h>
#include <qtstream.h>
#include <qapp.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qpainter.h>

/// QPixmap that may be loaded from an .xpm file.
/** QPixmap that may be loaded from an .xpm file. */
class KPixmap : public QPixmap
{
public:
  /// Default constructor.
  /** Default constructor. */
  KPixmap();

  /// Constructor with filename. 
  /** Constructor with filename. Loads a pixmap from the specified
	.xpm file */
  KPixmap( const char * );
  
  /// Destructor.
  /** Destructor. */
  virtual ~KPixmap() { }
    
  /// Load a pixmap from an .xpm file
  /** Load a pixmap from an .xpm file */
  virtual bool load( const char * );
  virtual bool loadXPM( const char *);
  virtual bool loadXVPICS( const char *);
 *
protected:
  QBitmap bitmap;
};
 * KPixmap - taken from kfm
 *
 * Author: Torben Weis
 *
 */

#ifndef KPIXMAP_H
#define KPIXMAP_H

#include <qpixmap.h>

#define KPixmap QPixmap

#endif
