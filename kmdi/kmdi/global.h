/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Based on:

   //----------------------------------------------------------------------------
   //    Project              : KDE MDI extension
   //
   //    begin                : 07/1999       by Szymon Stefanek as part of kvirc
   //                                         (an IRC application)
   //    changes              : 09/1999       by Falk Brettschneider to create an
   //                           - 06/2000     stand-alone Qt extension set of
   //                                         classes and a Qt-based library
   //                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
   //                           2000-2003     maintained by the KDevelop project
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#ifndef _KMDI_GLOBAL_H_
#define _KMDI_GLOBAL_H_

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

  /**
   * The style of the toolview tabs
   */
  enum ToolviewStyle {
   /** Show only icons on the toolview tabs. The visible toolviews contain both the icon and text. */
    IconOnly	= 0,
   /** Show only the text description on the toolview tabs. */
    TextOnly	= 1,
   /** Show both the icon and description on the toolview tabs. */
    TextAndIcon = 3
  };
}

#endif
