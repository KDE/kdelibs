/* This file is part of the KDE libraries
    Copyright (c) 2003 Luboš Luňák <l.lunak@kde.org>
    Copyright (c) 2012 Valentin Rusu <kde@rusu.info>

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

#ifndef KINTERPROCESSWINDOWING_KMESSAGEBOX_H
#define KINTERPROCESSWINDOWING_KMESSAGEBOX_H

#include <kmessagebox.h>
#include <kinterprocesswindowing_export.h>

namespace KMessageBox {

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int questionYesNoWId(WId parent_id,
                          const QString &text,
                          const QString &caption = QString(),
                          const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                          const KGuiItem &buttonNo =  KStandardGuiItem::no(),
                          const QString &dontAskAgainName = QString(),
                          Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT int questionYesNoCancelWId(WId parent_id,
                          const QString &text,
                          const QString &caption = QString(),
                          const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                          const KGuiItem &buttonNo = KStandardGuiItem::no(),
                          const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                          const QString &dontAskAgainName = QString(),
                          Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int questionYesNoListWId(WId parent_id,
                          const QString &text,
                          const QStringList &strlist,
                          const QString &caption = QString(),
                          const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                          const KGuiItem &buttonNo = KStandardGuiItem::no(),
                          const QString &dontAskAgainName = QString(),
                          Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int warningYesNoWId(WId parent_id,
                         const QString &text,
                         const QString &caption = QString(),
                         const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                         const KGuiItem &buttonNo = KStandardGuiItem::no(),
                         const QString &dontAskAgainName = QString(),
                         Options options = Options(Notify | Dangerous));

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int warningYesNoListWId(WId parent_id,
                            const QString &text,
                            const QStringList &strlist,
                            const QString &caption = QString(),
                            const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                            const KGuiItem &buttonNo = KStandardGuiItem::no(),
                            const QString &dontAskAgainName = QString(),
                            Options options = Options(Notify | Dangerous));

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int warningContinueCancelWId(WId parent_id,
                         const QString &text,
                         const QString &caption = QString(),
                         const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                         const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                         const QString &dontAskAgainName = QString(),
                         Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
 KINTERPROCESSWINDOWING_EXPORT int warningContinueCancelListWId(WId parent_id,
                         const QString &text,
                         const QStringList &strlist,
                         const QString &caption = QString(),
                         const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                         const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                         const QString &dontAskAgainName = QString(),
                         Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT int warningYesNoCancelWId(WId parent_id,
                                const QString &text,
                                const QString &caption = QString(),
                                const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                                const KGuiItem &buttonNo = KStandardGuiItem::no(),
                                const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                const QString &dontAskAgainName = QString(),
                                Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT int warningYesNoCancelListWId(WId parent_id,
                                const QString &text,
                                const QStringList &strlist,
                                const QString &caption = QString(),
                                const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                                const KGuiItem &buttonNo = KStandardGuiItem::no(),
                                const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                const QString &dontAskAgainName = QString(),
                                Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void errorWId(WId parent_id,
                    const QString &text,
                    const QString &caption = QString(),
                    Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */

  KINTERPROCESSWINDOWING_EXPORT void errorListWId(WId parent_id,
                    const QString &text,
                    const QStringList &strlist,
                    const QString &caption = QString(),
                    Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void detailedErrorWId(WId parent_id,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString(),
                    Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void sorryWId(WId parent_id,
                    const QString &text,
                    const QString &caption = QString(),
                    Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void detailedSorryWId(WId parent_id,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString(),
                    Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void informationWId(WId parent_id,
                          const QString &text,
                          const QString &caption = QString(),
                          const QString &dontShowAgainName = QString(),
                          Options options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of QWidget*. It should be used only when necessary.
  */
  KINTERPROCESSWINDOWING_EXPORT void informationListWId(WId parent_id,
                  const QString &text,
                  const QStringList & strlist,
                  const QString &caption = QString(),
                  const QString &dontShowAgainName = QString(),
                  Options options = Notify);

    /**
     * This function accepts the window id of the parent window, instead
     * of QWidget*. It should be used only when necessary.
     */
    KINTERPROCESSWINDOWING_EXPORT int messageBoxWId( WId parent_id, DialogType type, const QString &text,
                    const QString &caption = QString(),
                    const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                    const KGuiItem &buttonNo = KStandardGuiItem::no(),
                    const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                    const QString &dontShowAskAgainName = QString(),
                    Options options = Notify);


} // namespace

#endif // KINTERPROCESSWINDOWING_KMESSAGEBOX_H
