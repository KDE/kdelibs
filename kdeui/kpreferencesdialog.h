/*  This file is part of the KDE project
    Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>

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

// DESIGN:
//
// Things to consider:
// - Keep it simple and small. Every configuration page should be reachable
//   in the KPD. More than one dialog is bad.
// - Apps can load and unload different parts. It should always be clear how
//   to reach the preferences for those parts.
//   Since GUI merging already takes place (e.g. menu/toolbar entries are
//   added/removed) why not merge dialogs? Hmm actually it already happens:
//   the KEditToolbar Dialog changes when a different part is activated in
//   Konqueror.
//   Now, using Konqueror, if an image is shown, using the KView part, the
//   preferences for the part need to be available somewhere. Two
//   possibilities come to mind: An action is added to open a dialog for them
//   or the preferences dialog of the shell (Konqueror) includes the pages
//   from the part. The GUI changes - no way to avoid that. The only question
//   is: what is better? more logical?
//   Currently in Konqueror there's an action called "Configure Konqueror".
//   This gives you a dialog where you can configure the KHTML part and
//   Konqueror itself. Of course there is a pretty strong relationship between
//   the KHTML part and Konqueror but still it's not entirely logical.
//   Why would I want to configure the KHTML part preferences if I'm looking
//   at my home directory. I'd rather want to change the KonqDir part
//   settings. And if I open an image in Konqueror (using KView) and I open up
//   the Preferences Dialog I want to see the whole configuration for the
//   program I'm currently using - and that program currently is an image
//   viewer.
// - KOffice:
//   In KOffice it happens that you embed a part in a part. Since both parts
//   are available to you in the program you see, the preferences dialog should
//   include the configuration for both. If you then remove the embedded part,
//   so that only the first part remains, the preferences for that part
//   need to be removed from the dialog. I consider this part of the GUI
//   merging.
//
// - The problem with a singleton approach would be apps that show
//   multiple instances while only using one UNIX process. E.g. Konqueror
//   showing a Webpage and an image in two different windows sharing the
//   process would create a dialog with Konq, khtml and kview preferences.
// - Ideally every MainWindow would own a KPD. But how?
//
// How can a KPM find a KPD?
// - The KPM looks at all the QObject children of its parent. If there is no
//   KPD it looks at the children of that one's parent. If there is none, it
//   goes on so forth. If finally there's no parent left it needs to create a
//   new KPD. The KPD's parent is the parent of the KPM.
//
// Problem about all the finding in QObject hierarchie: If the app creates the
// KPD just before showing no KPMs will be registered.
// Plugin KPMs need to find the PluginSelector which is normally unavailable
// and if it is it's very hard to find (and probably impossible to find the
// right one)
//
#ifndef KPREFERENCESDIALOG_H
#define KPREFERENCESDIALOG_H

#include <qobject.h>

class KPreferencesModule;

/**
 * A Preference Dialog.
 *
 * This class makes it easy to create configuration/preference dialogs even
 * when using KParts and/or plugins which will merge their dialog pages into
 * the dialog automagically.
 *
 * Example usage:
 * </pre>
   MyApp::MyApp( QObject * parent, const char * name )
     : QObject( parent, name )
     , m_prefdlg( new KPreferencesDialog( this ) )
   {
   }

   void MyApp::slotPreferences()
   {
     m_prefdlg->show();
   }
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @version $Id$
 * @since 3.2
 */
class KPreferencesDialog : public QObject
{
	friend class KPreferencesModule;

	Q_OBJECT
	public:
		/**
		 * Construct a new Dialog object.
		 *
		 * @param parent  The parent object of this dialog. Normally this will
		 *                be your main widget (MainWindow).
		 */
		KPreferencesDialog( QObject * parent, const char * name = 0 );

		~KPreferencesDialog();

	public slots:
		/**
		 * Creates the dialog and shows it (it's not modal).
		 */
		void show();

	protected:
		void registerModule( KPreferencesModule * );
		void setGroupName( KPreferencesModule *, const QString & name );
		void createMenuEntry();

	protected slots:
		void unregisterModule( QObject * );
		void slotApply();
		void slotOk();
		void slotFinished();

	private:
		void createPage( KPreferencesModule * );

		class KPreferencesDialogPrivate;
		KPreferencesDialogPrivate * d;
};

// vim: sw=4 ts=4 tw=78
#endif // KPREFERENCESDIALOG_H
