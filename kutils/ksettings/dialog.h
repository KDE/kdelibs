/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KCONFIGUREDIALOG_H
#define KCONFIGUREDIALOG_H

#include <qobject.h>
#include <kservice.h>

/**
 * @short Generic configuration dialog that even works over component boundaries
 *
 * This class aims to standardize the use of configuration dialogs in KDE
 * applications. Especially when using KParts and/or Plugins you face problems
 * creating a consistent config dialog.
 *
 * To show a configuration dialog you only have to call the show method and be
 * done with it. A code example:
 *
 * You initialize \p m_cfgdlg with
 * \code
 * m_cfgdlg = new KConfigureDialog( this );
 * \endcode
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * \code
 * QStringList kpartslist;
 * for( all my kparts )
 *   kpartslist += m_mypart->instance().instanceName();
 * m_cfgdlg = new KConfigureDialog( kpartslist, this );
 * \endcode
 * and the action for the config dialog is connected to the show slot:
 * \code
 * KStdAction::preferences( m_cfgdlg, SLOT( show() ), actionCollection() );
 * \endcode
 *
 * If you need to be informed when the config was changed and applied in the
 * dialog you might want to take a look at KCDDispatcher.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KConfigureDialog : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Construct a new Preferences Dialog for the application. It uses all
		 * KCMs with X-KDE-ParentApp set to KGlobal::instance()->instanceName().
		 */
		KConfigureDialog( QObject * parent = 0, const char * name = 0 );

		/**
		 * Construct a new Preferences Dialog with the pages for the selected
		 * instance names. For example if you want to have the configuration
		 * pages for the kviewviewer KPart you would pass a
		 * QStringList consisting of only the name of the part "kviewviewer".
		 */
		KConfigureDialog( const QStringList & kcdparents, QObject * parent = 0, const char * name = 0 );

		//void addKPartsPluginPage();

		~KConfigureDialog();

	public slots:
		/**
		 * Show the config dialog. The slot immediatly returns since the dialog
		 * is non-modal.
		 */
		void show();

	private:
		QValueList<KService::Ptr> instanceServices() const;
		QValueList<KService::Ptr> parentComponentsServices( const QStringList & ) const;
		void createDialogFromServices( const QValueList<KService::Ptr> & );
		class KConfigureDialogPrivate;
		KConfigureDialogPrivate * d;
};

#endif // KCONFIGUREDIALOG_H

// vim: sw=4 ts=4
