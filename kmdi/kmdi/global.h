//----------------------------------------------------------------------------
//    filename             : kmdidefines.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------
#ifndef _KMDI_DEFINES_H_
#define _KMDI_DEFINES_H_

#include <qglobal.h>

#define KMDI_CHILDFRM_SEPARATOR 2
#define KMDI_CHILDFRM_BORDER 4
#define KMDI_CHILDFRM_DOUBLE_BORDER 8
#define KMDI_CHILDFRM_MIN_WIDTH 130

//----------------------------------------------------------------------------
/**
* @short A namespace for the KMDI library
*/
namespace KMDI
{
   enum TabWidgetVisibility {
      AlwaysShowTabs         = 0,
      ShowWhenMoreThanOneTab = 1,
      NeverShowTabs          = 2
   };

  /** The style of the toolview tabs
        @since 3.3
  */
   enum ToolviewStyle {
   /** Show only icons on the toolview tabs. The visible toolviews contain both the icon and text. */
      IconOnly	= 0,
   /** Show only the text description on the toolview tabs. */
      TextOnly	= 1,
   /** Show both the icon and description on the toolview tabs. */
      TextAndIcon = 3
   };
} //namespace

#endif //_KMDIDEFINES_H_
