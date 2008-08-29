// vim: noexpandtab ts=4 sw=4
/* This file is part of the KDE libraries
    Copyright (C) 2001, 2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef KKEYSEQUENCEWIDGET_H
#define KKEYSEQUENCEWIDGET_H

#include <QtCore/QList>
#include <QtGui/QPushButton>

#include <kshortcut.h>


class KKeySequenceWidgetPrivate;
class QAction;
class KActionCollection;

/**
 * @short A widget to input a QKeySequence.
 *
 * This widget lets the user choose a QKeySequence, which is usually used as a shortcut key,
 * by pressing the keys just like to trigger a shortcut. Calling captureKeySequence(), or
 * the user clicking into the widget, start recording.
 *
 * A check for conflict with shortcut of this application can also be performed.
 * call setCheckActionCollections() to set the list of action collections to check with,
 * and applyStealShortcut when applying changes.
 *
 * @author Mark Donohoe <donohoe@kde.org>
 * @internal
 */
class KDEUI_EXPORT KKeySequenceWidget: public QWidget
{
	Q_OBJECT

public:
	///An enum about validation when setting a key sequence.
	///@see setKeySequence()
	enum Validation {
		///Validate key sequence
		Validate = 0,
		///Use key sequence without validation
		NoValidate = 1
	};

	/**
	* Constructor.
	*/
	explicit KKeySequenceWidget(QWidget *parent = 0);

	/**
	* Destructs the widget.
	*/
	virtual ~KKeySequenceWidget();

	/**
	 * \group Configuration
	 *
	 * Configuration options for the widget.
	 */
	//@{

		enum ShortcutType {
			LocalShortcuts = 0x01,      //!< Check with local shortcuts. @see setCheckActionCollections()
			StandardShortcuts = 0x02,   //!< Check against standard shortcuts. @see KStandardShortcut
			GlobalShortcuts = 0x03      //!< Check against global shortcuts. @see KGlobalAccel
		};
		Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

		/**
		 * Configure if the widget should check for conflicts with existing
		 * shortcuts.
		 *
		 * When capturing a key sequence for local shortcuts you should check
		 * against GlobalShortcuts and your other local shortcuts. This is the
		 * default.
		 *
		 * The easiest way to check against local shortcuts is
		 * setCheckActionCollections().
		 *
		 * When capturing a key sequence for a global shortcut you should
		 * check against StandardShortcuts, GlobalShortcuts and your local
		 * shortcuts.
		 *
		 * If you want to do the conflict checking yourself here are some code
		 * snippets for standard and global shortcuts:
		 *
		 * \code
		 * QStringList conflicting = KGlobalAccel::findActionNameSystemwide(keySequence);
		 * if (!conflicting.isEmpty()) {
		 *     // Inform and ask the user about the conflict and reassigning
		 *     // the keys sequence
		 *     if (!KGlobalAccel::promptStealShortcutSystemwide(q, conflicting, keySequence)) {
		 *         return true;
		 *     }
		 *     KGlobalAccel::stealShortcutSystemwide(keySequence);
		 * }
		 * \endcode
		 *
		 * \code
		 * KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
		 * if (ssc != KStandardShortcut::AccelNone) {
		 *     // We have a conflict
		 * }
		 * \endcode
		 *
		 * We normally inform the user abput the possible conflict and let him
		 * proceed if he wants.
		 *
		 * @since 4.2
		 */
		void setCheckForConflictsAgainst( ShortcutTypes types );

		/**
		 * The shortcut types we check for conflicts.
		 * @see setCheckForConflictsAgainst
		 *
		 * @since 4.2
		 */
		ShortcutTypes checkForConflictsAgainst() const;

		/**
		 * This only applies to user input, not to setShortcut().
		 * Set whether to accept "plain" keys without modifiers (like Ctrl, Alt, Meta).
		 * Plain keys by our definition include letter and symbol keys and
		 * text editing keys (Return, Space, Tab, Backspace, Delete).
		 * "Special" keys like F1, Cursor keys, Insert, PageDown will always work.
		 */
		void setModifierlessAllowed(bool allow);

		/**
		 * @see setModifierlessAllowed()
		 */
		bool isModifierlessAllowed();

		/**
		 * Set whether a small button to set an empty key sequence should be displayed next to the
		 * main input widget. The default is to show the clear button.
		 */
		void setClearButtonShown(bool show);

	//@}

	/**
	 * Checks whether the key sequence @a seq is available to grab.
	 *
	 * The sequence is checked under the same rules as if it has been typed by
	 * the user. This method is useful if you get key sequences from another
	 * input source and want to check if it is save to set them.
	 *
	 * @since 4.2
	 */
	bool isKeySequenceAvailable(const QKeySequence &seq) const;

	/**
	 * Return the currently selected key sequence.
	 */
	QKeySequence keySequence() const;

	/**
	 * Set a list of action collections to check against for conflictuous shortcut.
	 *
	 * If a KAction with a conflicting shortcut is found inside this list and
	 * its shortcut can be configured (KAction::isShortcutConfigurable()
	 * returns true) the user will be prompted whether to steal the shortcut
	 * from this action.
	 *
	 * Global shortcuts are automatically checked for conflicts. For checking
	 * against KStandardShortcuts - @see checkAgainstStandardShortcuts().
	 *
	 * Don't forget to call applyStealShortcut to actually steal the shortcut
	 * and read it's documentation for some limitation when handling global
	 * shortcuts.
	 *
	 * @since 4.1
	 */
    void setCheckActionCollections(const QList<KActionCollection *>& actionCollections);

    /**
     * @deprecated since 4.1
     * use setCheckActionCollections so that KKeySequenceWidget knows
     * in which action collection to call the writeSettings method after stealing
     * a shortcut from an action.
     */
    KDE_DEPRECATED void setCheckActionList(const QList<QAction*> &checkList);

Q_SIGNALS:
	/**
	 * This signal is emitted when the current key sequence has changed, be it by user
	 * input or programmatically.
	 */
	void keySequenceChanged(const QKeySequence &seq);
public Q_SLOTS:
	/**
	 * Capture a shortcut from the keyboard. This call will only return once a key sequence
	 * has been captured or input was aborted.
	 * If a key sequence was input, keySequenceChanged() will be emitted.
	 *
	 * @see setModifierlessAllowed()
	 */
	void captureKeySequence();

	/**
	 * Set the key sequence.
	 *
	 * If @p val == Validate, and the call is actually changing the key sequence,
	 * conflictuous shortcut will be checked.
	 */
	void setKeySequence(const QKeySequence &seq, Validation val = NoValidate);

	/**
	 * Clear the key sequence.
	 */
	void clearKeySequence();

	/**
	 * Actually remove the shortcut that the user wanted to steal, from the
	 * action that was using it. This only applies to actions provided to us
	 * by setCheckActionCollections() and setCheckActionList().
	 *
	 * Global and Standard Shortcuts have to be stolen immediately when the
	 * user gives his consent (technical reasons). That means those changes
	 * will be active even if you never call applyStealShortcut().
	 *
	 * To be called before you apply your changes. No local shortcuts are
	 * stolen until this function is called.
	 */
	void applyStealShortcut();

private:
	Q_PRIVATE_SLOT(d, void doneRecording())

private:
	friend class KKeySequenceWidgetPrivate;
	KKeySequenceWidgetPrivate *const d;

	Q_DISABLE_COPY(KKeySequenceWidget)
};

#endif //KKEYSEQUENCEWIDGET_H
