/*
  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#ifndef KINPUTDIALOG_H
#define KINPUTDIALOG_H

#include <QtCore/QStringList>

#include <kdeui_export.h>

#include <limits.h>
#include <float.h>

class QValidator;

/**
 * The KInputDialog namespace provides simple dialogs to get a single value
 * from the user. The value can be a string, a number (either an integer or
 * a float) or an item from a list.
 *
 * @author Nadeem Hasan <nhasan@kde.org>
 */
namespace KInputDialog
{
    /**
     * Static convenience function to get a string from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as a label for the line edit. value is the initial
     * value of the line edit. ok will be set to true if user pressed Ok
     * and false if user pressed Cancel.
     *
     * If you provide a validator, the Ok button is disabled as long as
     * the validator doesn't return Acceptable. If there is no validator,
     * the Ok button is enabled whenever the line edit isn't empty. If you
     * want to accept empty input, create a trivial QValidator that
     * always returns acceptable, e.g. QRegExpValidator with a regexp
     * of ".*".
     *
     * @param caption   Caption of the dialog
     * @param label     Text of the label for the line edit
     * @param value     Initial value of the line edit
     * @param ok        This bool would be set to true if user pressed Ok
     * @param parent    Parent of the dialog widget
     * @param validator A @ref QValidator to be associated with the line edit
     * @param mask      Mask associated with the line edit. See the
     *                  documentation for @ref QLineEdit about masks
     * @param whatsThis a QWhatsThis text for the input widget.
     * @param completionList a list of items which should be used for input completion
     * @return String user entered if Ok was pressed, else a null string
     */
    KDEUI_EXPORT QString getText( const QString &caption, const QString &label,
                                  const QString &value=QString(), bool *ok=0, QWidget *parent=0,
                                  QValidator *validator=0,
                                  const QString &mask=QString(),
                                  const QString& whatsThis=QString(),
                                  const QStringList &completionList=QStringList()  );

    /**
     * Static convenience function to get a multiline string from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as a label for the line edit. value is the initial
     * value of the line edit. ok will be set to true if user pressed Ok
     * and false if user pressed Cancel.
     *
     * @param caption   Caption of the dialog
     * @param label     Text of the label for the line edit
     * @param value     Initial value of the line edit
     * @param ok        This bool would be set to true if user pressed Ok
     * @param parent    Parent of the dialog widget
     *
     * @return String user entered if Ok was pressed, else a null string
     */
    KDEUI_EXPORT QString getMultiLineText( const QString &caption,
                                           const QString &label, const QString &value=QString(),
                                           bool *ok=0, QWidget *parent=0 );

    /**
     * Static convenience function to get an integer from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as the label for the spin box. value is the initial
     * value for the spin box. minValue and maxValue are the minimum and
     * maximum allowable values the user may choose. step is the amount by
     * which the value will change as the user presses the increment and
     * decrement buttons of the spin box. Base is the base of the number.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param value    Initial value of the spin box
     * @param minValue Minimum value user can input
     * @param maxValue Maximum value user can input
     * @param step     Amount by which value is incremented or decremented
     * @param base     Base of the number
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     *
     * @return Number user entered if Ok was pressed, else 0
     */

    KDEUI_EXPORT int getInteger( const QString &caption, const QString &label,
                                 int value=0, int minValue=INT_MIN, int maxValue=INT_MAX,
                                 int step=1, int base=10, bool *ok=0, QWidget *parent=0 );

    /**
     * This is an overloaded convenience function. It behaves exactly same as
     * above except it assumes base to be 10, i.e. accepts decimal numbers.
     */
    KDEUI_EXPORT int getInteger( const QString &caption, const QString &label,
                                 int value=0, int minValue=INT_MIN, int maxValue=INT_MAX,
                                 int step=1, bool *ok=0, QWidget *parent=0 );

    /**
     * Static convenience function to get a floating point number from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as the label for the spin box. value is the initial
     * value for the spin box. minValue and maxValue are the minimum and
     * maximum allowable values the user may choose. step is the amount by
     * which the value will change as the user presses the increment and
     * decrement buttons of the spin box.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param value    Initial value of the spin box
     * @param minValue Minimum value user can input
     * @param maxValue Maximum value user can input
     * @param step     Amount by which value is incremented or decremented
     * @param decimals Number of digits after the decimal point
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     *
     * @return Number user entered if Ok was pressed, else 0
     */
    KDEUI_EXPORT double getDouble( const QString &caption, const QString &label,
                                   double value=0, double minValue=-DBL_MAX,
                                   double maxValue=DBL_MAX, double step=0.1, int decimals=1,
                                   bool *ok=0, QWidget *parent=0 );

    /**
     * This is an overloaded convenience function. It behaves exctly like
     * the above function.
     */
    KDEUI_EXPORT double getDouble( const QString &caption, const QString &label,
                                   double value=0, double minValue=-DBL_MAX,
                                   double maxValue=DBL_MAX, int decimals=1, bool *ok=0,
                                   QWidget *parent=0 );

    /**
     * Static convenience function to let the user select an item from a
     * list. caption is the text that is displayed in the title bar.
     * label is the text that appears as the label for the list. list
     * is the string list which is inserted into the list, and current
     * is the number of the item which should be the selected item. If
     * editable is true, the user can enter his own text.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the list
     * @param list     List of item for user to choose from
     * @param current  Index of the selected item
     * @param editable If true, user can enter own text
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     *
     * @return Text of the selected item. If @p editable is true this can be
     *         a text entered by the user.
     */
    KDEUI_EXPORT QString getItem( const QString &caption, const QString &label,
                                  const QStringList &list, int current=0, bool editable=false,
                                  bool *ok=0, QWidget *parent=0 );

    /**
     * Static convenience function to let the user select one or more
     * items from a listbox. caption is the text that is displayed in the
     * title bar. label is the text that appears as the label for the listbox.
     * list is the string list which is inserted into the listbox, select
     * is the list of item(s) that should be the selected. If multiple is
     * true, the user can select multiple items.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the list
     * @param list     List of item for user to choose from
     * @param select   List of item(s) that should be selected
     * @param multiple If true, user can select multiple items
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     *
     * @return List of selected items if multiple is true, else currently
     *         selected item as a QStringList
     */
    KDEUI_EXPORT QStringList getItemList( const QString &caption,
                                          const QString &label, const QStringList &list=QStringList(),
                                          const QStringList &select=QStringList(), bool multiple=false,
                                          bool *ok=0, QWidget *parent=0 );
}

#endif // KINPUTDIALOG_H

/* vim: set ai et sw=2 ts=2
*/
