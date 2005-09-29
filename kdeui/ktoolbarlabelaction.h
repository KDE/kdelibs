/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KTOOLBARLABELACTION_H
#define KTOOLBARLABELACTION_H

#include <kactionclasses.h>

class QLabel;

/**
 * @short Class to display a label in a toolbar.
 *
 * KToolBarLabelAction is a convenience class for displaying a label in a
 * toolbar.
 *
 * It provides easy access to the label's #setBuddy(QWidget*) and #buddy()
 * methods and can be used as follows:
 *
 * \code
 *
 * KHistoryCombo* findCombo = new KHistoryCombo(true, this);
 * KWidgetAction* action
 *   = new KWidgetAction(findCombo, i18n("F&ind Combo"), Qt::Key_F6, this,
 *                       SLOT(slotFocus()), actionCollection(), "find_combo");
 *
 * new KToolBarLabelAction(findCombo, i18n("F&ind "), 0, this,
 *                         SLOT(slotFocus()), actionCollection(),
 *             "find_label");
 *
 * \endcode
 *
 * @author Felix Berger <felixberger@beldesign.de>
 */
class KDEUI_EXPORT KToolBarLabelAction : public KWidgetAction
{
public:
  /**
   * Constructs a toolbar label.
   *
   * @param text The label's and the action's text.
   * @param cut The action's shortcut.
   * @param receiver The SLOT's parent.
   * @param slot The SLOT to invoke to execute this action.
   * @param parent This action's parent.
   * @param name An internal name for this action.
   */
  KToolBarLabelAction(const QString &text,
		      const KShortcut &cut,
		      const QObject *receiver, const char *slot,
		      KActionCollection *parent, const char *name);
  /**
   * Constructs a toolbar label setting a buddy for the label.
   *
   * @param buddy The widget which is focused when the label's accelerator is
   * typed.
   * @param text The label's and the action's text.
   * @param cut The action's shortcut.
   * @param receiver The SLOT's parent.
   * @param slot The SLOT to invoke to execute this action.
   * @param parent This action's parent.
   * @param name An internal name for this action.
   */
  KToolBarLabelAction(QWidget* buddy, const QString &text,
		      const KShortcut &cut,
		      const QObject *receiver, const char *slot,
		      KActionCollection *parent, const char *name);
  /**
   * Constructs a toolbar label for a label.
   *
   * You can use this constructor if you want to display a class which is
   * derived from QLabel in the toolbar. Note that ownership of the label is
   * transferred to the action and the label is deleted when the action is
   * deleted. So you shouldn't hold any pointers to the label.
   *
   * It's important that the label's name is set to "kde toolbar widget" in
   * its constructor, otherwise it is not correctly rendered in some kde
   * styles.
   *
   * @param label the label which is displayed in the toolbar.
   * @param cut The action's shortcut.
   * @param receiver The SLOT's parent.
   * @param slot The SLOT to invoke to execute this action.
   * @param parent This action's parent.
   * @param name An internal name for this action.
   */
  KToolBarLabelAction(QLabel* label, const KShortcut &cut, 
		      const QObject *receiver, const char *slot,
		      KActionCollection* parent, const char *name);

  virtual ~KToolBarLabelAction();
  /**
   * Reimplemented to update both the action's text and the label's text.
   */
  virtual void setText(const QString& text);
  /**
   * Sets the label's buddy to buddy.
   *
   * See QLabel#setBuddy() for details.
   */
  virtual void setBuddy(QWidget* buddy);
  /**
   * Returns the label's buddy or 0 if no buddy is currently set.
   *
   * See QLabel#buddy() and QLabel#setBuddy() for more information.
   */
  QWidget* buddy() const;
  /**
   * Returns the label which is used internally.
   */
  QLabel* label() const;

protected:
  virtual void virtual_hook(int id, void* data);

private:
  class KToolBarLabelActionPrivate;
  KToolBarLabelActionPrivate *d;
  void init();
};


#endif
