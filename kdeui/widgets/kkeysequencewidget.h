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
 * @short A push button that looks like a keyboard key.
 *
 * You must call setShortcut() to set the widget's currently displayed key.
 * You can call captureShortcut() to get a new shortcut from the user.
 * If captureShortcut() succeeds, then the capturedShortcut() signal will be
 * emitted with the value of the new shortcut.  The widget containing
 * a KKeySequenceWidget widget must connect to this signal and check if the shortcut
 * is valid.  If it is, you will need to call setShortcut() with the new
 * value in order make it the key currently displayed.
 *
 * @author Mark Donohoe <donohoe@kde.org>
 * @internal
 */
class KDEUI_EXPORT KKeySequenceWidget: public QWidget
{
	Q_OBJECT

public:
	/**
	* Constructor.
	*/
	explicit KKeySequenceWidget(QWidget *parent = 0);
	/**
	* Destructs the widget.
	*/
	virtual ~KKeySequenceWidget();

	void setModifierlessAllowed(bool allow);
	bool isModifierlessAllowed();
	
	void setHaveClearButton(bool show);

	QKeySequence keySequence() const;

Q_SIGNALS:
	void keySequenceChanged(const QKeySequence &seq);

public Q_SLOTS:
	/**
	 * Call this method to capture a shortcut from the keyboard.
	 * If it succeeds, keySequenceChanged() will be emitted.
	 */
	void captureKeySequence();
    void setKeySequence(const QKeySequence &seq);
    void clearKeySequence();

private:
	Q_PRIVATE_SLOT(d, void doneRecording());

private:
	//TODO: can these be moved in the private class? //remark: yes, use emit q->blah!
	void startRecording();
	friend class KKeySequenceWidgetPrivate;
	KKeySequenceWidgetPrivate *const d;

	Q_DISABLE_COPY(KKeySequenceWidget)
};

#endif //KKEYSEQUENCEWIDGET_H
