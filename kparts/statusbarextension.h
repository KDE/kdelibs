/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef KPARTS_STATUSBAREXTENSION_H
#define KPARTS_STATUSBAREXTENSION_H

#include <qwidget.h>
#include <qvaluelist.h>

class KStatusBar;
class KMainWindow;

namespace KParts
{

  class ReadOnlyPart;
  class GUIActivateEvent;

  // Defined in impl
  class StatusBarItem;
  

  /**
   * @short an extension for KParts that allows more sophisticated statusbar handling
   *`
   * Every part can use this class to customize the statusbar as long as it is active.
   * add items via @p addStatusBarItem and remove it with @p removeStatusBarItem.
   *
   * You can inherit from this class and reimplement @p mainWindow for custom usage.
   **/
  class StatusBarExtension : public QObject
  {
    Q_OBJECT
   
    public:
      StatusBarExtension( KParts::ReadOnlyPart *parent, const char *name=0L );
      ~StatusBarExtension();

      /**
       * This adds a widget to the statusbar for this part.
       * If you use this method instead of using statusBar() directly,
       * this exentension will take care of removing the items when the parts GUI
       * is deactivated and readding them when it is reactivated.
       * The parameters are the same as @ref QStatusBar::addWidget().
       *
       * Note that you can't use KStatusBar methods (inserting text items by id).
       * But you can create a KStatusBarLabel with a dummy id instead, and use
       * it directly, to get the same look and feel.
       */
      void addStatusBarItem( QWidget * widget, int stretch, bool permanent );

      /**
       * Remove a widget from the statusbar for this part.
       */
      void removeStatusBarItem( QWidget * widget );

      /**
       * @return the KMainWindow in which this part is currently embedded.
       * This one should never return 0L, in a KDE app.
       *
       * You might want to reimplement this if your mainWindow is in a different place.
       */
      virtual KMainWindow* mainWindow() const;

      /**
       * @return the statusbar of the KMainWindow in which this part is currently embedded.
       * WARNING: this could return 0L
       */
      KStatusBar* statusBar() const;

      /** @internal */
      void eventFilter( KParts::ReadOnlyPart *watched, GUIActivateEvent* ev );

    private:
     QValueList<StatusBarItem> m_statusBarItems; // Our statusbar items

     // for future extensions
     class StatusBarExtensionPrivate;
     StatusBarExtensionPrivate *d;
  };

};
#endif // KPARTS_STATUSBAREXTENSION_H

// vim: ts=2 sw=2 et
