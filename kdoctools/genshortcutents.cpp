/*
 * genshortcutents.cpp - Copyright 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "kaboutdata.h"
#include "kcomponentdata.h"
#include "kcmdlineargs.h"
#include "klocale.h"
#include "kstandardshortcut.h"

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>

static QString keyAsMarkup( const QString &key )
{
	if ( key == "Alt" || key == "Ctrl" || key == "Enter" || key == "Esc" ||
	     key == "Shift" || key == "Tab" ) {
		return QString('&' + key + ';');
	}
	if ( key == "Left" || key == "Right" || key == "Up" || key == "Down" ) {
		return QString("<keysym>" + key + " Arrow</keysym>");
	}
	if ( key == "Backspace" || key == "Menu" ) {
		return QString("<keysym>" + key + "</keysym>");
	}
	if ( key == "Plus" ) {
		return QString("<keycap>+</keycap>");
	}
	if ( key == "Minus" ) {
		return QString("<keycap>-</keycap>");
	}
	return QString("<keycap>" + key + "</keycap>");
}

static QString entityForAccel( KStandardShortcut::StandardShortcut accel )
{
	QString markup = "<!ENTITY KeyCombo_";

	/* We use a switch so that the compiler gives us warning messages in
	 * case KStandardShortcut::StandardShortcut gets extended (which means we have to
	 * extend this code, too).
	 */
	switch ( accel ) {
		case KStandardShortcut::AccelNone:
			break;
		case KStandardShortcut::Open:
			markup += "Open";
			break;
		case KStandardShortcut::OpenRecent:
			markup += "OpenRecent";
			break;
		case KStandardShortcut::New:
			markup += "New";
			break;
		case KStandardShortcut::Close:
			markup += "Close";
			break;
		case KStandardShortcut::Save:
			markup += "Save";
			break;
		case KStandardShortcut::SaveAs:
			markup += "SaveAs";
			break;
		case KStandardShortcut::Print:
			markup += "Print";
			break;
		case KStandardShortcut::Quit:
			markup += "Quit";
			break;
		case KStandardShortcut::Undo:
			markup += "Undo";
			break;
		case KStandardShortcut::Redo:
			markup += "Redo";
			break;
		case KStandardShortcut::Cut:
			markup += "Cut";
			break;
		case KStandardShortcut::Copy:
			markup += "Copy";
			break;
		case KStandardShortcut::Paste:
			markup += "Paste";
			break;
		case KStandardShortcut::SelectAll:
			markup += "SelectAll";
			break;
		case KStandardShortcut::Deselect:
			markup += "Deselect";
			break;
		case KStandardShortcut::DeleteWordBack:
			markup += "DeleteWordBack";
			break;
		case KStandardShortcut::DeleteWordForward:
			markup += "DeleteWordForward";
			break;
		case KStandardShortcut::Find:
			markup += "Find";
			break;
		case KStandardShortcut::FindNext:
			markup += "FindNext";
			break;
		case KStandardShortcut::FindPrev:
			markup += "FindPrev";
			break;
		case KStandardShortcut::Replace:
			markup += "Replace";
			break;
		case KStandardShortcut::Home:
			markup += "Home";
			break;
		case KStandardShortcut::Begin:
			markup += "Begin";
			break;
		case KStandardShortcut::End:
			markup += "End";
			break;
		case KStandardShortcut::Prior:
			markup += "Prior";
			break;
		case KStandardShortcut::Next:
			markup += "Next";
			break;
		case KStandardShortcut::GotoLine:
			markup += "GotoLine";
			break;
		case KStandardShortcut::AddBookmark:
			markup += "AddBookmark";
			break;
		case KStandardShortcut::ZoomIn:
			markup += "ZoomIn";
			break;
		case KStandardShortcut::ZoomOut:
			markup += "ZoomOut";
			break;
		case KStandardShortcut::Up:
			markup += "Up";
			break;
		case KStandardShortcut::Back:
			markup += "Back";
			break;
		case KStandardShortcut::Forward:
			markup += "Forward";
			break;
		case KStandardShortcut::Reload:
			markup += "Reload";
			break;
		case KStandardShortcut::ShowMenubar:
			markup += "ShowMenubar";
			break;
		case KStandardShortcut::Help:
			markup += "Help";
			break;
		case KStandardShortcut::WhatsThis:
			markup += "WhatsThis";
			break;
		case KStandardShortcut::TextCompletion:
			markup += "TextCompletion";
			break;
		case KStandardShortcut::PrevCompletion:
			markup += "PrevCompletion";
			break;
		case KStandardShortcut::NextCompletion:
			markup += "NextCompletion";
			break;
		case KStandardShortcut::SubstringCompletion:
			markup += "SubstringCompletion";
			break;
		case KStandardShortcut::RotateUp:
			markup += "RotateUp";
			break;
		case KStandardShortcut::RotateDown:
			markup += "RotateDown";
			break;
		case KStandardShortcut::TabNext:
			markup += "TabNext";
			break;
		case KStandardShortcut::TabPrev:
			markup += "TabPrev";
			break;
		case KStandardShortcut::FullScreen:
			markup += "FullScreen";
			break;
		case KStandardShortcut::BackwardWord:
			markup += "BackwardWord";
			break;
		case KStandardShortcut::ForwardWord:
			markup += "ForwardWord";
			break;
		case KStandardShortcut::BeginningOfLine:
			markup += "BeginningOfLine";
			break;
		case KStandardShortcut::EndOfLine:
			markup += "EndOfLine";
			break;
		case KStandardShortcut::PasteSelection:
			markup += "PastSelection";
			break;
		case KStandardShortcut::Revert:
			markup += "Revert";
			break;
		case KStandardShortcut::PrintPreview:
			markup += "PrintPreview";
			break;
		case KStandardShortcut::Mail:
			markup += "Mail";
			break;
		case KStandardShortcut::Clear:
			markup += "Clear";
			break;
		case KStandardShortcut::ActualSize:
			markup += "ActualSize";
			break;
		case KStandardShortcut::FitToPage:
			markup += "FitToPage";
			break;
		case KStandardShortcut::FitToWidth:
			markup += "FitToWidth";
			break;
		case KStandardShortcut::FitToHeight:
			markup += "FitToHeight";
			break;
		case KStandardShortcut::Zoom:
			markup += "Zoom";
			break;
		case KStandardShortcut::Goto:
			markup += "Goto";
			break;
		case KStandardShortcut::GotoPage:
			markup += "GotoPage";
			break;
		case KStandardShortcut::DocumentBack:
			markup += "DocumentBack";
			break;
		case KStandardShortcut::DocumentForward:
			markup += "DocumentForward";
			break;
		case KStandardShortcut::EditBookmarks:
			markup += "EditBookmarks";
			break;
		case KStandardShortcut::Spelling:
			markup += "Spelling";
			break;
		case KStandardShortcut::ShowToolbar:
			markup += "ShowToolbar";
			break;
		case KStandardShortcut::ShowStatusbar:
			markup += "ShowStatusbar";
			break;
		case KStandardShortcut::SaveOptions:
			markup += "SaveOptions";
			break;
		case KStandardShortcut::KeyBindings:
			markup += "KeyBindings";
			break;
		case KStandardShortcut::Preferences:
			markup += "Preferences";
			break;
		case KStandardShortcut::ConfigureToolbars:
			markup += "ConfigureToolbars";
			break;
		case KStandardShortcut::ConfigureNotifications:
			markup += "ConfigureNotifications";
			break;
		case KStandardShortcut::TipofDay:
			markup += "TipofDay";
			break;
		case KStandardShortcut::ReportBug:
			markup += "ReportBug";
			break;
		case KStandardShortcut::SwitchApplicationLanguage:
			markup += "SwitchApplicationLanguage";
			break;
		case KStandardShortcut::AboutApp:
			markup += "AboutApp";
			break;
		case KStandardShortcut::AboutKDE:
			markup += "AboutKDE";
			break;
		case KStandardShortcut::StandardShortcutCount:
			break;
	}

	markup += "\t\"";

	QString internalStr = KStandardShortcut::shortcut( accel ).toString();
	QString firstSequence = internalStr.left( internalStr.indexOf( ';' ) );
	const QStringList keys = firstSequence.split( '+',QString::SkipEmptyParts );
	if ( keys.empty() ) {
		return QString();
	}

	if ( keys.count() == 1 ) {
		if ( keys.first().startsWith( QLatin1String("XF86") ) ) {
			return QString();
		}
		markup += keyAsMarkup( keys.first() );
	} else {
		markup += "<keycombo action=\"Simul\">";
		foreach( const QString &key, keys ) {
			if ( key.startsWith( QLatin1String("XF86") ) ) {
				continue;
			}
			markup += keyAsMarkup( key );
		}
		markup += "</keycombo>";
	}
	markup += "\">";
	return markup;
}

int main( int argc, char **argv )
{
        KCmdLineOptions cmdLineOptions;
	cmdLineOptions.add("o");
	cmdLineOptions.add("output <file>", ki18n("Output file"), "kde-standard-accels.entities");

	KAboutData aboutData( "genshortcutents", 0, ki18n("genshortcutents"), "1.0",
	                      ki18n( "Generates DocBook entities for key shortcuts of standard actions" ));

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( cmdLineOptions );

	KComponentData componentData( &aboutData );
	QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	const QString outputFileName = args->getOption( "output" );
	QFile outputFile( outputFileName );
	if ( !outputFile.open( QIODevice::WriteOnly ) ) {
		qDebug( "Failed to open %s for writing.", qPrintable( outputFileName ) );
		return 1;
	}

	QTextStream stream( &outputFile );
	stream << "<!-- Generated by "
	       << aboutData.programName() + ' '
	       << aboutData.version() + " on "
	       << QDateTime::currentDateTime().toString() + ".\n";
	stream << "     Do not bother to fiddle with this, your modifications will be\n"
	       << "     overwritten as soon as the file is regenerated. -->\n";
	stream << "\n";

	for ( unsigned int i = 0; i < KStandardShortcut::StandardShortcutCount; ++i ) {
            stream << entityForAccel( static_cast<KStandardShortcut::StandardShortcut>(i)  ) << "\n";
	}
}

