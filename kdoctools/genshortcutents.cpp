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
#include "kapplication.h"
#include "kcmdlineargs.h"
#include "klocale.h"
#include "kstdaccel.h"

#include <qdatetime.h>
#include <qfile.h>
//Added by qt3to4:
#include <QTextStream>
#include <QStringList>
static const KCmdLineOptions cmdLineOptions[] = {
	{ "o", 0, 0 },
	{ "output <file>", I18N_NOOP( "Output file" ), "kde-standard-accels.entities" },
	KCmdLineLastOption
};

static KStdAccel::StdAccel accelerators[] = {
	KStdAccel::Open,
	KStdAccel::New,
	KStdAccel::Close,
	KStdAccel::Save,
	KStdAccel::Print,
	KStdAccel::Quit,
	KStdAccel::Undo,
	KStdAccel::Redo,
	KStdAccel::Cut,
	KStdAccel::Copy,
	KStdAccel::Paste,
	KStdAccel::SelectAll,
	KStdAccel::Deselect,
	KStdAccel::DeleteWordBack,
	KStdAccel::DeleteWordForward,
	KStdAccel::Find,
	KStdAccel::FindNext,
	KStdAccel::FindPrev,
	KStdAccel::Replace,
	KStdAccel::Home,
	KStdAccel::End,
	KStdAccel::Prior,
	KStdAccel::Next,
	KStdAccel::GotoLine,
	KStdAccel::AddBookmark,
	KStdAccel::ZoomIn,
	KStdAccel::ZoomOut,
	KStdAccel::Up,
	KStdAccel::Back,
	KStdAccel::Forward,
	KStdAccel::Reload,
	KStdAccel::PopupMenuContext,
	KStdAccel::ShowMenubar,
	KStdAccel::Help,
	KStdAccel::WhatsThis,
	KStdAccel::TextCompletion,
	KStdAccel::PrevCompletion,
	KStdAccel::NextCompletion,
	KStdAccel::SubstringCompletion,
	KStdAccel::RotateUp,
	KStdAccel::RotateDown,
	KStdAccel::TabNext,
	KStdAccel::TabPrev,
	KStdAccel::FullScreen,
	KStdAccel::BackwardWord,
	KStdAccel::ForwardWord,
	KStdAccel::BeginningOfLine,
	KStdAccel::EndOfLine,
	KStdAccel::PasteSelection
};
static const unsigned int numAccelerators = sizeof( accelerators ) / sizeof( accelerators[ 0 ] );

QString keyAsMarkup( const QString &key )
{
	if ( key == "Alt" || key == "Ctrl" || key == "Enter" || key == "Esc" ||
	     key == "Shift" || key == "Tab" ) {
		return QString() + "&" + key + ";";
	}
	if ( key == "Left" || key == "Right" || key == "Up" || key == "Down" ) {
		return QString() + "<keysym>" + key + " Arrow</keysym>";
	}
	if ( key == "Backspace" || key == "Menu" ) {
		return QString() + "<keysym>" + key + "</keysym>";
	}
	if ( key == "Plus" ) {
		return "<keycap>+</keycap>";
	}
	if ( key == "Minus" ) {
		return "<keycap>-</keycap>";
	}
	return QString() + "<keycap>" + key + "</keycap>";
}

QString entityForAccel( KStdAccel::StdAccel accel )
{
	QString markup = "<!ENTITY KeyCombo_";

	/* We use a switch so that the compiler gives us warning messages in
	 * case KStdAccel::StdAccel gets extended (which means we have to
	 * extend this code, too).
	 */
	switch ( accel ) {
		case KStdAccel::AccelNone:
			break;
		case KStdAccel::Open:
			markup += "Open";
			break;
		case KStdAccel::New:
			markup += "New";
			break;
		case KStdAccel::Close:
			markup += "Close";
			break;
		case KStdAccel::Save:
			markup += "Save";
			break;
		case KStdAccel::Print:
			markup += "Print";
			break;
		case KStdAccel::Quit:
			markup += "Quit";
			break;
		case KStdAccel::Undo:
			markup += "Undo";
			break;
		case KStdAccel::Redo:
			markup += "Redo";
			break;
		case KStdAccel::Cut:
			markup += "Cut";
			break;
		case KStdAccel::Copy:
			markup += "Copy";
			break;
		case KStdAccel::Paste:
			markup += "Paste";
			break;
		case KStdAccel::SelectAll:
			markup += "SelectAll";
			break;
		case KStdAccel::Deselect:
			markup += "Deselect";
			break;
		case KStdAccel::DeleteWordBack:
			markup += "DeleteWordBack";
			break;
		case KStdAccel::DeleteWordForward:
			markup += "DeleteWordForward";
			break;
		case KStdAccel::Find:
			markup += "Find";
			break;
		case KStdAccel::FindNext:
			markup += "FindNext";
			break;
		case KStdAccel::FindPrev:
			markup += "FindPrev";
			break;
		case KStdAccel::Replace:
			markup += "Replace";
			break;
		case KStdAccel::Home:
			markup += "Home";
			break;
		case KStdAccel::End:
			markup += "End";
			break;
		case KStdAccel::Prior:
			markup += "Prior";
			break;
		case KStdAccel::Next:
			markup += "Next";
			break;
		case KStdAccel::GotoLine:
			markup += "GotoLine";
			break;
		case KStdAccel::AddBookmark:
			markup += "AddBookmark";
			break;
		case KStdAccel::ZoomIn:
			markup += "ZoomIn";
			break;
		case KStdAccel::ZoomOut:
			markup += "ZoomOut";
			break;
		case KStdAccel::Up:
			markup += "Up";
			break;
		case KStdAccel::Back:
			markup += "Back";
			break;
		case KStdAccel::Forward:
			markup += "Forward";
			break;
		case KStdAccel::Reload:
			markup += "Reload";
			break;
		case KStdAccel::PopupMenuContext:
			markup += "PopupMenuContext";
			break;
		case KStdAccel::ShowMenubar:
			markup += "ShowMenubar";
			break;
		case KStdAccel::Help:
			markup += "Help";
			break;
		case KStdAccel::WhatsThis:
			markup += "WhatsThis";
			break;
		case KStdAccel::TextCompletion:
			markup += "TextCompletion";
			break;
		case KStdAccel::PrevCompletion:
			markup += "PrevCompletion";
			break;
		case KStdAccel::NextCompletion:
			markup += "NextCompletion";
			break;
		case KStdAccel::SubstringCompletion:
			markup += "SubstringCompletion";
			break;
		case KStdAccel::RotateUp:
			markup += "RotateUp";
			break;
		case KStdAccel::RotateDown:
			markup += "RotateDown";
			break;
		case KStdAccel::TabNext:
			markup += "TabNext";
			break;
		case KStdAccel::TabPrev:
			markup += "TabPrev";
			break;
		case KStdAccel::FullScreen:
			markup += "FullScreen";
			break;
		case KStdAccel::BackwardWord:
			markup += "BackwardWord";
			break;
		case KStdAccel::ForwardWord:
			markup += "ForwardWord";
			break;
		case KStdAccel::BeginningOfLine:
			markup += "BeginningOfLine";
			break;
		case KStdAccel::EndOfLine:
			markup += "EndOfLine";
			break;
		case KStdAccel::PasteSelection:
			markup += "PastSelection";
			break;
	}

	markup += "\t\"";

	QString internalStr = KStdAccel::shortcut( accel ).toStringInternal();
	QString firstSequence = internalStr.left( internalStr.indexOf( ';' ) );
	const QStringList keys = firstSequence.split( '+',QString::SkipEmptyParts );
	if ( keys.empty() ) {
		return QString();
	}

	if ( keys.count() == 1 ) {
		if ( keys.first().startsWith( "XF86" ) ) {
			return QString();
		}
		markup += keyAsMarkup( keys.first() );
	} else {
		markup += "<keycombo action=\"Simul\">";
		foreach( QString key, keys ) {
			if ( key.startsWith( "XF86" ) ) {
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
	KCmdLineArgs::init( argc, argv, "genshortcutents", "genshortcutents",
	                    I18N_NOOP( "Generates DocBook entities for key shortcuts of standard actions" ),
	                    "1.0" );
	KCmdLineArgs::addCmdLineOptions( cmdLineOptions );

	KApplication::disableAutoDcopRegistration();
	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	const QString outputFileName = args->getOption( "output" );
	QFile outputFile( outputFileName );
	if ( !outputFile.open( QIODevice::WriteOnly ) ) {
		qDebug( "Failed to open %s for writing.", qPrintable( outputFileName ) );
		return 1;
	}

	QTextStream stream( &outputFile );
	stream << "<!-- Generated by "
	       << app.aboutData()->programName() + " "
	       << app.aboutData()->version() + " on "
	       << QDateTime::currentDateTime().toString() + ".\n";
	stream << "     Don't bother to fiddle with this, your modifications will be\n"
	       << "     overwritten as soon as the file is regenerated. -->\n";
	stream << "\n";

	for ( unsigned int i = 0; i < numAccelerators; ++i ) {
		stream << entityForAccel( accelerators[ i ] ) << "\n";
	}
}

