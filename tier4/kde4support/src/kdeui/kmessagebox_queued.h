/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

class QWidget;

#include <QString>
#include <qwindowdefs.h>

#include <kmessagebox.h>

#include <kde4support_export.h>

namespace KMessageBox {

  /**
   * Like detailedError
   *
   * This function will return immediately, the messagebox will be shown
   * once the application enters an event loop and no other messagebox
   * is being shown.
   *
   * Note that if the parent gets deleted, the messagebox will not be
   * shown.
   */
  KDE4SUPPORT_DEPRECATED_EXPORT void queuedDetailedError( QWidget *parent,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString());

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KDE4SUPPORT_DEPRECATED_EXPORT void queuedDetailedErrorWId( WId parent_id,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString());


    /**
     * Like messageBox
     *
     * Only for message boxes of type Information, Sorry or Error.
     *
     * This function will return immediately, the messagebox will be shown
     * once the application enters an event loop and no other messagebox
     * is being shown.
     *
     * Note that if the parent gets deleted, the messagebox will not be
     * shown.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void queuedMessageBox( QWidget *parent,
                    DialogType type, const QString &text,
                    const QString &caption,
                    Options options );

    /**
     * This function accepts the window id of the parent window, instead
     * of QWidget*. It should be used only when necessary.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void queuedMessageBoxWId( WId parent_id,
                    DialogType type, const QString &text,
                    const QString &caption,
                    Options options );

    /**
     * @overload
     *
     * This is an overloaded member function, provided for convenience.
     * It behaves essentially like the above function.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void queuedMessageBox( QWidget *parent,
                    DialogType type, const QString &text,
                    const QString &caption = QString() );

    /**
     * This function accepts the window id of the parent window, instead
     * of QWidget*. It should be used only when necessary.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void queuedMessageBoxWId( WId parent_id,
                    DialogType type, const QString &text,
                    const QString &caption = QString() );

} // namespace
