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

#include <QtGui/QPushButton>

#include <kshortcut.h>


class KKeySequenceWidgetPrivate;


/**
 * @short A widget to input a QKeySequence.
 *
 * This widget lets the user choose a QKeySequence, which is usually used as a shortcut key,
 * by pressing the keys just like to trigger a shortcut. Calling captureKeySequence(), or
 * the user clicking into the widget, start recording.
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
	 * Set whether to accept plain letter or symbol keys without modifiers like Ctrl, Alt, Meta.
	 * "Special" keys like F1, Insert, PageDown will always work.
	 * This only applies to user input, not to setShortcut().
	 */
	void setModifierlessAllowed(bool allow);

	/**
	 * Return if the widget accepts plain letter or symbol keys without modifiers like Ctrl, Alt, Meta.
	 * "Special" keys like F1, Insert and so on will always work.
	 */
	bool isModifierlessAllowed();

	/**
	 * Set whether a small button to set an empty key sequence should be displayed next to the
	 * main input widget. The default is to show the clear button.
	 */
	void setClearButtonShown(bool show);

	/**
	 * Return the currently selected key sequence.
	 */
	QKeySequence keySequence() const;

Q_SIGNALS:
	/**
	 * This signal is emitted when the current key sequence has changed, be it by user
	 * input or programmatically.
	 */
	void keySequenceChanged(const QKeySequence &seq);

	/**
	 * This signal is emitted when the key sequence has changed as the result of user input or
	 * calling setKeySequence(seq, Validate). Call denyValidation() in a slot called from this
	 * signal to deny a change of key sequence.
	 * Do not call setKeySequence() in a slot called from this signal. Do it later if you have to.
	 */
	void validationHook(const QKeySequence &newSeq);

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
	 * If @p val == Validate (the default), and the call is actually changing the key
	 * sequence, the signal validationHook() will be emitted.
	 */
	void setKeySequence(const QKeySequence &seq, Validation val = Validate);
	
	/**
	 * Clear the key sequence.
	 */
	void clearKeySequence();

	/**
	 * @see validationHook().
	 */
	void denyValidation();

private:
	Q_PRIVATE_SLOT(d, void doneRecording())

private:
	friend class KKeySequenceWidgetPrivate;
	KKeySequenceWidgetPrivate *const d;

	Q_DISABLE_COPY(KKeySequenceWidget)
};

#endif //KKEYSEQUENCEWIDGET_H
