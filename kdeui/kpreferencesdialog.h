/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KPREFERENCESDIALOG_H
#define KPREFERENCESDIALOG_H

#include <qobject.h>

class KPreferencesModule;
class KDialogBase;

/**
 * A @ref KDialogBase that eases the creation of a Preferences Dialog.
 *
 * To use it you need to create some @ref KPreferencesModule objects which will
 * automatically insert themselves into the dialog. When it comes to showing the
 * dialog to the user you call KPreferencesDialog::show().
 * The dialog is always non-modal. (What good is an apply button if the dialog
 * is modal?)
 *
 * Everytime the user presses Apply or OK the @ref reloadConfig() signal is
 * emitted. You can either connect to this signal and then reload the
 * configuration from the config object your @ref KPreferencesModule wrote to or
 * the module applies the new settings in it's save() method.
 *
 * @author Matthias Kretz (kretz@kde.org)
 * @version $Id$
 */
class KPreferencesDialog : public QObject
{
	friend class KPreferencesModule;

	Q_OBJECT
	public:
		virtual ~KPreferencesDialog();

		/**
		 * Shows a modeless dialog. Control returns immediately to the calling code. 
		 */
		static void show();

		/**
		 * Sets the help path and topic.
		 *
		 * @param anchor Defined anchor in your docbook sources
		 * @param appname Defines the appname the help belongs to
		 *                If empty it's the current one
		 */
		static void setHelp( const QString &anchor, const QString &appname = QString::null );

		/**
		 * Access to the Object. You need this to be able to connect the signals
		 * from this object to your slots.
		 */
		static KPreferencesDialog * self();

	signals:
		/**
		 * Emitted when the user pressed Apply or OK. You can connect to this
		 * signal to reload your configuration after the changes in the dialog
		 * are committed.
		 */
		void reloadConfig();

	protected:
		void showInternal();

		void addModule( KPreferencesModule * );
		void removeModule( KPreferencesModule * );
		//void addModule( KCModule * );
		//void removeModule( KCModule * );

	protected slots:
		void slotApply();
		void slotOk();

	private:
		KPreferencesDialog();

		class KPreferencesDialogPrivate;
		KPreferencesDialogPrivate * d;

		static KPreferencesDialog * m_instance;
		KDialogBase * m_dialog;
};

// vim:sw=4:ts=4

#endif // KPREFERENCESDIALOG_H
