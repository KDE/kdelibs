/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 David Faure <faure@kde.org>

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
 */

#ifndef KPARTS_STATUSBAREXTENSION_H
#define KPARTS_STATUSBAREXTENSION_H

#include <QtGui/QWidget>

#include <kparts/kparts_export.h>
#include <QtCore/QList>

class KStatusBar;
class KMainWindow;
class QEvent;

namespace KParts
{

  class ReadOnlyPart;

  // Defined in impl
  class StatusBarItem;
  class StatusBarExtensionPrivate;


  /**
   * @short an extension for KParts that allows more sophisticated statusbar handling
   *
   * Every part can use this class to customize the statusbar as long as it is active.
   * Add items via addStatusBarItem and remove an item with removeStatusBarItem.
   *
   * IMPORTANT: do NOT add any items immediately after constructing the extension.
   * Give the application time to set the statusbar in the extension if necessary.
   */
  class KPARTS_EXPORT StatusBarExtension : public QObject
  {
    Q_OBJECT

    public:
      StatusBarExtension( KParts::ReadOnlyPart *parent );
      ~StatusBarExtension();

      /**
       * This adds a widget to the statusbar for this part.
       * If you use this method instead of using statusBar() directly,
       * this extension will take care of removing the items when the parts GUI
       * is deactivated and will re-add them when it is reactivated.
       * The parameters are the same as QStatusBar::addWidget().
       *
       * Note that you can't use KStatusBar methods (inserting text items by id)
       * but you can create a KStatusBarLabel with a dummy id instead, and use
       * it directly in order to get the same look and feel.
       *
       * @param widget the widget to add
       * @param stretch the stretch factor. 0 for a minimum size.
       * @param permanent passed to QStatusBar::addWidget as the "permanent" bool.
       * Note that the item isn't really permanent though, it goes away when
       * the part is unactivated. This simply controls where temporary messages
       * hide the @p widget, and whether it's added to the left or to the right side.
       *
       * IMPORTANT: do NOT add any items immediately after constructing the extension.
       * Give the application time to set the statusbar in the extension if necessary.
       */
      void addStatusBarItem( QWidget * widget, int stretch, bool permanent );

      /**
       * Remove a widget from the statusbar for this part.
       */
      void removeStatusBarItem( QWidget * widget );

      /**
       * @return the statusbar of the KMainWindow in which this part is currently embedded.
       * WARNING: this could return 0L
       */
      KStatusBar* statusBar() const;

      /**
       * This allows the hosting application to set a particular KStatusBar
       * for this part. If it doesn't do this, the statusbar used will be
       * the one of the KMainWindow in which the part is embedded.
       * Konqueror uses this to assign a view-statusbar to the part.
       * The part should never call this method!
       */
      void setStatusBar( KStatusBar* status );

      /**
       * Queries @p obj for a child object which inherits from this
       * BrowserExtension class. Convenience method.
       */
      static StatusBarExtension *childObject( QObject *obj );

      /** @internal */
      virtual bool eventFilter( QObject *watched, QEvent* ev );

    private:
     // for future extensions
     friend class StatusBarExtensionPrivate;
     StatusBarExtensionPrivate* const d;
  };

}
#endif // KPARTS_STATUSBAREXTENSION_H

// vim: ts=2 sw=2 et
