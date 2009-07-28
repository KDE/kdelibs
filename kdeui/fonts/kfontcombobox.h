/*  This file is part of the KDE libraries

    Copyright (C) 2008 Chusslove Illich <caslav.ilic@gmx.net>

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
#ifndef KFONTCOMBOBOX_P_H
#define KFONTCOMBOBOX_P_H

#include <kdeui_export.h>

#include <kcombobox.h>

class KFontComboBoxPrivate;

/**
 * @short A lightweight font selection widget.
 *
 * A combobox to select the font from. Lightweight counterpart to KFontChooser,
 * for situations where only the font family should be selected, while the
 * font style and size are handled by other means. Like in KFontChooser,
 * this widget will show the font previews in the unrolled dropdown list.
 *
 * @note The class is similar to QFontComboBox, but more tightly integrated
 * with KDE desktop. Use it instead of QFontComboBox by default in KDE code.
 *
 * \image html kfontcombobox.png "KDE Font Combo Box"
 *
 * @author Chusslove Illich \<caslav.ilic@gmx.net\>
 *
 * @see KFontAction
 * @see KFontChooser
 *
 * @since 4.1
 */
class KDEUI_EXPORT KFontComboBox : public KComboBox
{
    Q_OBJECT

    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont)

public:

    /**
     * Constructor.
     *
     * @param parent the parent widget
     */
    explicit KFontComboBox (QWidget *parent = 0);

    /**
     * Toggle selectable fonts to be only those of fixed width or all.
     *
     * @param onlyFixed only fixed width fonts when @p true,
     *                  all fonts when @p false
     */
    void setOnlyFixed (bool onlyFixed);

    /**
     * Destructor.
     */
    virtual ~KFontComboBox ();

    /**
     * The font currently selected from the list.
     *
     * @return the selected font
     */
    QFont currentFont () const;

    /**
     * The recommended size of the widget.
     * Reimplemented to make the recommended width independent
     * of the particular fonts installed.
     *
     * @return recommended size
     */
    virtual QSize sizeHint() const;

public Q_SLOTS:
    /**
     * Set the font to show as selected in the combobox.
     *
     * @param font the new font
     */
    void setCurrentFont (const QFont &font);

Q_SIGNALS:
    /**
     * Emitted when a new font has been selected,
     * either through user input or by setFont().
     *
     * @param font the new font
     */
    void currentFontChanged (const QFont &font);

protected:
    bool event (QEvent *e);

private:

    friend class KFontComboBoxPrivate;
    KFontComboBoxPrivate * const d;

    Q_DISABLE_COPY(KFontComboBox)

    Q_PRIVATE_SLOT(d, void _k_currentFontChanged (int))
};

#endif
