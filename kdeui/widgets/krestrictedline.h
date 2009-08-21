/*
 * Definition of KRestrictedLine
 *
 * Copyright (C) 1997 Michael Wiedmann, <mw@miwie.in-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KRESTRICTEDLINE_H
#define KRESTRICTEDLINE_H

#include <klineedit.h>

class KRestrictedLinePrivate;
/**
 * @short A line editor for restricted character sets.
 *
 * The KRestrictedLine widget is a variant of QLineEdit which
 * accepts only a restricted set of characters as input.
 * All other characters will be discarded and the signal invalidChar()
 * will be emitted for each of them.
 *
 * Valid characters can be passed as a QString to the constructor
 * or set afterwards via setValidChars().
 * The default key bindings of QLineEdit are still in effect.
 *
 * This is almost like setting a QRegExpValidator on a KLineEdit;
 * the difference is that with KRestrictedLine it can all be done in Qt designer.
 *
 * \image html krestrictedline.png "KDE Restricted Line Edit allowing all characters but 'o'"
 *
 * @author Michael Wiedmann <mw@miwie.in-berlin.de>
 */
class KDEUI_EXPORT KRestrictedLine : public KLineEdit
{
  Q_OBJECT
  Q_PROPERTY( QString validChars READ validChars WRITE setValidChars )

public:

  /**
   * Constructor
   *  @param parent   pointer to the parent widget
   */
  explicit KRestrictedLine( QWidget* parent = 0);

  /**
   * Destructs the restricted line editor.
   */
  ~KRestrictedLine();

  /**
   * All characters in the string valid are treated as
   * acceptable characters.
   */
  void setValidChars(const QString& valid);
  /**
   * @return the string of acceptable characters.
   */
  QString validChars() const;

Q_SIGNALS:

  /**
   * Emitted when an invalid character was typed.
   */
  void	invalidChar(int);

protected:
    void keyPressEvent( QKeyEvent *e );
    void inputMethodEvent(QInputMethodEvent *e);

private:
  KRestrictedLinePrivate * const d;
};

#endif // KRESTRICTEDLINE_H
