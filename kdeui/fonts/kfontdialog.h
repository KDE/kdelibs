/*
    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

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
#ifndef K_FONT_DIALOG_H
#define K_FONT_DIALOG_H

#include <kdialog.h>
#include <kfontchooser.h>

class QFont;
class QStringList;

/**
 * @short A font selection dialog.
 *
 * The KFontDialog provides a dialog for interactive font selection.
 * It is basically a thin wrapper around the KFontChooser widget,
 * which can also be used standalone. In most cases, the simplest
 * use of this class is the static method KFontDialog::getFont(),
 * which pops up the dialog, allows the user to select a font, and
 * returns when the dialog is closed.
 *
 * Example:
 *
 * \code
 *      QFont myFont;
 *      int result = KFontDialog::getFont( myFont );
 *      if ( result == KFontDialog::Accepted )
 *            ...
 * \endcode
 *
 * \image html kfontdialog.png "KDE Font Dialog"
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 */
class KDEUI_EXPORT KFontDialog : public KDialog  {
    Q_OBJECT

public:
  /**
   * Constructs a font selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param flags Defines how the font chooser is displayed.
   *        @see KFontChooser::DisplayFlags
   * @param onlyFixed only display fonts which have fixed-width
   *        character sizes.
   * @param fontlist a list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param makeFrame Draws a frame with titles around the contents.
   * @param diff Display the difference version dialog. See getFontDiff().
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState, user choice may be retrieved by
   *        calling sizeIsRelative().
   *
   */
  explicit KFontDialog( QWidget *parent = 0,
                        const KFontChooser::DisplayFlags& flags =
                            KFontChooser::NoDisplayFlags,
                        const QStringList &fontlist = QStringList(),
                        Qt::CheckState *sizeIsRelativeState = 0 );

  ~KFontDialog();
  /**
   * Sets the currently selected font in the dialog.
   *
   * @param font The font to select.
   * @param onlyFixed readjust the font list to display only fixed
   *        width fonts if true, or vice-versa
   */
  void setFont( const QFont &font, bool onlyFixed = false );

  /**
   * @return The currently selected font in the dialog.
   */
  QFont font() const;

  /**
   * Sets the state of the checkbox indicating whether the font size
   * is to be interpreted as relative size.
   * NOTE: If parameter sizeIsRelative was not set in the constructor
   *       of the dialog this setting will be ignored.
   */
  void setSizeIsRelative( Qt::CheckState relative );

  /**
   * @return Whether the font size is to be interpreted as relative size
   *         (default: false)
   */
  Qt::CheckState sizeIsRelative() const;

  /**
   * Creates a modal font dialog, lets the user choose a font,
   * and returns when the dialog is closed.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param flags Defines how the font chooser is displayed.
   *        @see KFontChooser::DisplayFlags
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @return QDialog::result().
   */
  static int getFont( QFont &theFont,
                      const KFontChooser::DisplayFlags& flags =
                        KFontChooser::NoDisplayFlags,
                      QWidget *parent = 0L,
                      Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * Creates a modal font difference dialog, lets the user choose a selection
   * of changes that should be made to a set of fonts, and returns when the
   * dialog is closed. Useful for choosing slight adjustments to the font set
   * when the user would otherwise have to manually edit a number of fonts.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param flags  Defines how the font chooser is displayed.
   *        @see KFontChooser::DisplayFlags
   * @param diffFlags a reference to the int into which the chosen
   *        difference selection bitmask should be written.
   *        Check the returned bitmask like:
   *        \code
   *        if ( diffFlags & KFontChooser::FontDiffFamily )
   *            [...]
   *        if ( diffFlags & KFontChooser::FontDiffStyle )
   *            [...]
   *        if ( diffFlags & KFontChooser::FontDiffSize )
   *            [...]
   *        \endcode
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @returns QDialog::result().
   */
  static int getFontDiff( QFont &theFont,
                          KFontChooser::FontDiffFlags& diffFlags,
                          const KFontChooser::DisplayFlags& flags =
                                KFontChooser::NoDisplayFlags,
                          QWidget *parent = 0L,
                          Qt::CheckState *sizeIsRelativeState = 0L );

  /**
   * When you are not only interested in the font selected, but also
   * in the example string typed in, you can call this method.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param theString a reference to the example text that was typed.
   * @param flags  Defines how the font chooser is displayed.
   *        @see KFontChooser::DisplayFlags
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   * @return The result of the dialog.
   */
  static int getFontAndText( QFont &theFont, QString &theString,
                             const KFontChooser::DisplayFlags& flags =
                                KFontChooser::NoDisplayFlags,
                             QWidget *parent = 0L,
                             Qt::CheckState *sizeIsRelativeState = 0L );

Q_SIGNALS:
  /**
   * Emitted whenever the currently selected font changes.
   * Connect to this to monitor the font as it is selected if you are
   * not running modal.
   */
  void fontSelected( const QFont &font );

private:
  class Private;
  Private * const d;

  Q_DISABLE_COPY(KFontDialog)
};

#endif
