/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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
#include "kstdaction.h"

#include <qtoolbutton.h>
#include <qwhatsthis.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstdaccel.h>

namespace KStdAction
{

struct KStdActionInfo
{
	StdAction id;
	KStdAccel::StdAccel idAccel;
	const char* psName;
	const char* psLabel;
	const char* psWhatsThis;
	const char* psIconName;
};

KStdActionInfo g_rgActionInfo[] =
{
	{ New,           KStdAccel::New, "file_new", I18N_NOOP("&New"), 0, "filenew" },
	{ Open,          KStdAccel::Open, "file_open", I18N_NOOP("&Open..."), 0, "fileopen" },
	{ OpenRecent,    KStdAccel::AccelNone, "file_open_recent", I18N_NOOP("Open &Recent"), 0, 0 },
	{ Save,          KStdAccel::Save, "file_save", I18N_NOOP("&Save"), 0, "filesave" },
	{ SaveAs,        KStdAccel::AccelNone, "file_save_as", I18N_NOOP("Save &As..."), 0, "filesaveas" },
	{ Revert,        KStdAccel::AccelNone, "file_revert", I18N_NOOP("Re&vert"), 0, "revert" },
	{ Close,         KStdAccel::Close, "file_close", I18N_NOOP("&Close"), 0, "fileclose" },
	{ Print,         KStdAccel::Print, "file_print", I18N_NOOP("&Print..."), 0, "fileprint" },
	{ PrintPreview,  KStdAccel::AccelNone, "file_print_preview", I18N_NOOP("Print Previe&w..."), 0, "filequickprint" },
	{ Mail,          KStdAccel::AccelNone, "file_mail", I18N_NOOP("&Mail..."), 0, "mail_send" },
	{ Quit,          KStdAccel::Quit, "file_quit", I18N_NOOP("&Quit"), 0, "exit" },

	{ Undo,          KStdAccel::Undo, "edit_undo", I18N_NOOP("&Undo"), 0, "undo" },
	{ Redo,          KStdAccel::Redo, "edit_redo", I18N_NOOP("Re&do"), 0, "redo" },
	{ Cut,           KStdAccel::Cut, "edit_cut", I18N_NOOP("Cu&t"), 0, "editcut" },
	{ Copy,          KStdAccel::Copy, "edit_copy", I18N_NOOP("&Copy"), 0, "editcopy" },
	{ Paste,         KStdAccel::Paste, "edit_paste", I18N_NOOP("&Paste"), 0, "editpaste" },
	{ SelectAll,     KStdAccel::SelectAll, "edit_select_all", I18N_NOOP("Select &All"), 0, 0 },
	{ Deselect,      KStdAccel::Deselect, "edit_deselect", I18N_NOOP("Dese&lect"), 0, 0 },
	{ Find,          KStdAccel::Find, "edit_find", I18N_NOOP("&Find..."), 0, "find" },
	{ FindNext,      KStdAccel::FindNext, "edit_find_next", I18N_NOOP("Find &Next"), 0, "next" },
	{ FindPrev,      KStdAccel::FindPrev, "edit_find_last", I18N_NOOP("Find &Previous"), 0, "previous" },
	{ Replace,       KStdAccel::Replace, "edit_replace", I18N_NOOP("&Replace..."), 0, 0 },

	{ ActualSize,    KStdAccel::AccelNone, "view_actual_size", I18N_NOOP("&Actual Size"), 0, 0 },
	{ FitToPage,     KStdAccel::AccelNone, "view_fit_to_page", I18N_NOOP("&Fit To Page"), 0, 0 },
	{ FitToWidth,    KStdAccel::AccelNone, "view_fit_to_width", I18N_NOOP("Fit To Page &Width"), 0, 0 },
	{ FitToHeight,   KStdAccel::AccelNone, "view_fit_to_height", I18N_NOOP("Fit To Page &Height"), 0, 0 },
	{ ZoomIn,        KStdAccel::ZoomIn, "view_zoom_in", I18N_NOOP("Zoom &In"), 0, "viewmag+" },
	{ ZoomOut,       KStdAccel::ZoomOut, "view_zoom_out", I18N_NOOP("Zoom &Out"), 0, "viewmag-" },
	{ Zoom,          KStdAccel::AccelNone, "view_zoom", I18N_NOOP("&Zoom..."), 0, "viewmag" },
	{ Redisplay,     KStdAccel::AccelNone, "view_redisplay", I18N_NOOP("&Redisplay"), 0, "reload" },

	{ Up,            KStdAccel::Up, "go_up", I18N_NOOP("&Up"), 0, "up" },
	// The following three have special i18n() needs for sLabel
	{ Back,          KStdAccel::Back, "go_back", 0, 0, "back" },
	{ Forward,       KStdAccel::Forward, "go_forward", 0, 0, "forward" },
	{ Home,          KStdAccel::Home, "go_home", 0, 0, "gohome" },
	{ Prior,         KStdAccel::Prior, "go_previous", I18N_NOOP("&Previous Page"), 0, "previous" },
	{ Next,          KStdAccel::Next, "go_next", I18N_NOOP("&Next Page"), 0, "next" },
	{ Goto,          KStdAccel::AccelNone, "go_goto", I18N_NOOP("&Go to..."), 0, 0 },
	{ GotoPage,      KStdAccel::AccelNone, "go_goto_page", I18N_NOOP("&Go to Page..."), 0, "goto" },
	{ GotoLine,      KStdAccel::GotoLine, "go_goto_line", I18N_NOOP("&Go to Line..."), 0, 0 },
	{ FirstPage,     KStdAccel::AccelNone, "go_first", I18N_NOOP("&First Page"), 0, "top" },
	{ LastPage,      KStdAccel::AccelNone, "go_last", I18N_NOOP("&Last Page"), 0, "bottom" },

	{ AddBookmark,   KStdAccel::AddBookmark, "bookmark_add", I18N_NOOP("&Add Bookmark"), 0, "bookmark_add" },
	{ EditBookmarks, KStdAccel::AccelNone, "bookmark_edit", I18N_NOOP("&Edit Bookmarks"), 0, "bookmark" },

	{ Spelling,      KStdAccel::AccelNone, "tools_spelling", I18N_NOOP("&Spelling..."), 0, "spellcheck" },

	{ ShowMenubar,   KStdAccel::ShowMenubar, "options_show_menubar", I18N_NOOP("Show &Menubar"), 0, "showmenu" },
	{ ShowToolbar,   KStdAccel::AccelNone, "options_show_toolbar", I18N_NOOP("Show &Toolbar"), 0, 0 },
	{ ShowStatusbar, KStdAccel::AccelNone, "options_show_statusbar", I18N_NOOP("Show St&atusbar"), 0, 0 },
	{ SaveOptions,   KStdAccel::AccelNone, "options_save_options", I18N_NOOP("&Save Settings"), 0, 0 },
	{ KeyBindings,   KStdAccel::AccelNone, "options_configure_keybinding", I18N_NOOP("Configure S&hortcuts..."), 0, "key_bindings" },
	{ Preferences,   KStdAccel::AccelNone, "options_configure", I18N_NOOP("&Configure %1..."), 0, "configure" },
	{ ConfigureToolbars, KStdAccel::AccelNone, "options_configure_toolbars", I18N_NOOP("Configure Tool&bars..."), 0, "style" },

	{ Help,          KStdAccel::Help, "help", 0, 0, "help" },
	{ HelpContents,  KStdAccel::AccelNone, "help_contents", I18N_NOOP(""), 0, "contents" },
	{ WhatsThis,     KStdAccel::WhatsThis, "help_whats_this", I18N_NOOP("What's &This?"), 0, "contexthelp" },
	{ ReportBug,     KStdAccel::AccelNone, "help_report_bug", I18N_NOOP("&Report Bug..."), 0, 0 },
	{ AboutApp,      KStdAccel::AccelNone, "help_about_app", 0, 0, 0 },
	{ AboutKDE,      KStdAccel::AccelNone, "help_about_kde", I18N_NOOP("About &KDE"), 0, "go" },
	{ ActionNone, KStdAccel::AccelNone, 0, 0, 0, 0 }
};

static KStdActionInfo* infoPtr( StdAction id )
{
	for( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ ) {
		if( g_rgActionInfo[i].id == id )
			return &g_rgActionInfo[i];
	}
	return 0;
}

KAction* create( StdAction id, KActionCollection* parent,
	const QObject *recvr, const char *slot, const char *name )
{
	kdDebug(125) << "KStdAction::create( " << id << ", " << parent << ", " << name << " )" << endl; // ellis
	KStdActionInfo* pInfo = infoPtr( id );
	if( pInfo ) {
		QString sLabel;
		switch( id ) {
			case Back: sLabel = i18n("go back", "&Back"); break;
			case Forward: sLabel = i18n("go forward", "&Forward"); break;
			case Home: sLabel = i18n("beginning (of line)", "&Home"); break;
			case Help: sLabel = i18n("show help", "&Help"); break;
			case HelpContents:
				{
				const KAboutData *aboutData = KGlobal::instance()->aboutData();
				QString appName = (aboutData) ? aboutData->programName() : QString::fromLatin1(kapp->name());
				sLabel = i18n("%1 &Handbook").arg(appName);
				}
				break;
			case AboutApp:
				{
				const KAboutData *aboutData = KGlobal::instance()->aboutData();
				QString appName = (aboutData) ? aboutData->programName() : QString::fromLatin1(kapp->name());
				sLabel = i18n("&About %1").arg(appName);
				}
				break;
			default: sLabel = i18n(pInfo->psLabel);
		}

		return new KAction( sLabel, pInfo->psIconName,
			KStdAccel::shortcut(pInfo->idAccel),
			recvr, slot,
			parent, (name) ? name : pInfo->psName );
	}
	return 0;
}

KAction *action(StdAction act_enum, const QObject *recvr,
                            const char *slot, QObject *parent, const char *name )
{
    KAction *act;
    switch (act_enum)
    {
    case New:
        act = openNew(recvr, slot, parent, name);
        break;
    case Open:
        act = open(recvr, slot, parent, name);
        break;
    case OpenRecent:
        act = openRecent(recvr, slot, parent, name);
        break;
    case Save:
        act = save(recvr, slot, parent, name);
        break;
    case SaveAs:
        act = saveAs(recvr, slot, parent, name);
        break;
    case Revert:
        act = revert(recvr, slot, parent, name);
        break;
    case Close:
        act = close(recvr, slot, parent, name);
        break;
    case Print:
        act = print(recvr, slot, parent, name);
        break;
    case PrintPreview:
        act = printPreview(recvr, slot, parent, name);
        break;
    case Mail:
        act = mail(recvr, slot, parent, name);
        break;
    case Quit:
        act = quit(recvr, slot, parent, name);
        break;

    case Undo:
        act = undo(recvr, slot, parent, name);
        break;
    case Redo:
        act = redo(recvr, slot, parent, name);
        break;
    case Cut:
        act = cut(recvr, slot, parent, name);
        break;
    case Copy:
        act = copy(recvr, slot, parent, name);
        break;
    case Paste:
        act = paste(recvr, slot, parent, name);
        break;
    case SelectAll:
        act = selectAll(recvr, slot, parent, name);
        break;
    case Deselect:
        act = deselect(recvr, slot, parent, name);
        break;
    case Find:
        act = find(recvr, slot, parent, name);
        break;
    case FindNext:
        act = findNext(recvr, slot, parent, name);
        break;
    case FindPrev:
        act = findPrev(recvr, slot, parent, name);
        break;
    case Replace:
        act = replace(recvr, slot, parent, name);
        break;

    case ActualSize:
        act = actualSize(recvr, slot, parent, name);
        break;
    case FitToPage:
        act = fitToPage(recvr, slot, parent, name);
        break;
    case FitToWidth:
        act = fitToWidth(recvr, slot, parent, name);
        break;
    case FitToHeight:
        act = fitToHeight(recvr, slot, parent, name);
        break;
    case ZoomIn:
        act = zoomIn(recvr, slot, parent, name);
        break;
    case ZoomOut:
        act = zoomOut(recvr, slot, parent, name);
        break;
    case Zoom:
        act = zoom(recvr, slot, parent, name);
        break;
    case Redisplay:
        act = redisplay(recvr, slot, parent, name);
        break;

    case Up:
        act = up(recvr, slot, parent, name);
        break;
    case Back:
        act = back(recvr, slot, parent, name);
        break;
    case Forward:
        act = forward(recvr, slot, parent, name);
        break;
    case Home:
        act = home(recvr, slot, parent, name);
        break;
    case Prior:
        act = prior(recvr, slot, parent, name);
        break;
    case Next:
        act = next(recvr, slot, parent, name);
        break;
    case Goto:
        act = goTo(recvr, slot, parent, name);
        break;
    case GotoPage:
        act = gotoPage(recvr, slot, parent, name);
        break;
    case GotoLine:
        act = gotoLine(recvr, slot, parent, name);
        break;
    case FirstPage:
        act = firstPage(recvr, slot, parent, name);
        break;
    case LastPage:
        act = lastPage(recvr, slot, parent, name);
        break;

    case AddBookmark:
        act = addBookmark(recvr, slot, parent, name);
        break;
    case EditBookmarks:
        act = editBookmarks(recvr, slot, parent, name);
        break;

    case Spelling:
        act = spelling(recvr, slot, parent, name);
        break;

    case ShowMenubar:
        act = showMenubar(recvr, slot, parent, name);
        break;
    case ShowToolbar:
        act = showToolbar(recvr, slot, parent, name);
        break;
    case ShowStatusbar:
        act = showStatusbar(recvr, slot, parent, name);
        break;
    case SaveOptions:
        act = saveOptions(recvr, slot, parent, name);
        break;
    case KeyBindings:
        act = keyBindings(recvr, slot, parent, name);
        break;
    case Preferences:
        act = preferences(recvr, slot, parent, name);
        break;
    case ConfigureToolbars:
        act = configureToolbars(recvr, slot, parent, name);
        break;

    case Help:
        act = help(recvr, slot, parent, name);
        break;
    case HelpContents:
        act = helpContents(recvr, slot, parent, name);
        break;
    case WhatsThis:
        act = whatsThis(recvr, slot, parent, name);
        break;
    case ReportBug:
        act = reportBug(recvr, slot, parent, name);
        break;
    case AboutApp:
        act = aboutApp(recvr, slot, parent, name);
        break;
    case AboutKDE:
        act = aboutKDE(recvr, slot, parent, name);
        break;

    default:
        act = 0;
    }

    return act;
}

const char* stdName(StdAction act_enum)
{
    const char *ret;
    switch (act_enum)
    {
    case New:
        ret = "file_new";
        break;
    case Open:
        ret = "file_open";
        break;
    case OpenRecent:
        ret = "file_open_recent";
        break;
    case Save:
        ret = "file_save";
        break;
    case SaveAs:
        ret = "file_save_as";
        break;
    case Revert:
        ret = "file_revert";
        break;
    case Close:
        ret = "file_close";
        break;
    case Print:
        ret = "file_print";
        break;
    case PrintPreview:
        ret = "file_print_preview";
        break;
    case Mail:
        ret = "file_mail";
        break;
    case Quit:
        ret = "file_quit";
        break;

    case Undo:
        ret = "edit_undo";
        break;
    case Redo:
        ret = "edit_redo";
        break;
    case Cut:
        ret = "edit_cut";
        break;
    case Copy:
        ret = "edit_copy";
        break;
    case Paste:
        ret = "edit_paste";
        break;
    case SelectAll:
        ret = "edit_select_all";
        break;
    case Deselect:
        ret = "edit_deselect";
        break;
    case Find:
        ret = "edit_find";
        break;
    case FindNext:
        ret = "edit_find_next";
        break;
    case FindPrev:
        ret = "edit_find_last";
        break;
    case Replace:
        ret = "edit_replace";
        break;

    case ActualSize:
        ret = "view_actual_size";
        break;
    case FitToPage:
        ret = "view_fit_to_page";
        break;
    case FitToWidth:
        ret = "view_fit_to_width";
        break;
    case FitToHeight:
        ret = "view_fit_to_height";
        break;
    case ZoomIn:
        ret = "view_zoom_in";
        break;
    case ZoomOut:
        ret = "view_zoom_out";
        break;
    case Zoom:
        ret = "view_zoom";
        break;
    case Redisplay:
        ret = "view_redisplay";
        break;

    case Up:
        ret = "go_up";
        break;
    case Back:
        ret = "go_back";
        break;
    case Forward:
        ret = "go_forward";
        break;
    case Home:
        ret = "go_home";
        break;
    case Prior:
        ret = "go_previous";
        break;
    case Next:
        ret = "go_next";
        break;
    case Goto:
        ret = "go_goto";
        break;
    case GotoPage:
        ret = "go_goto_page";
        break;
    case GotoLine:
        ret = "go_goto_line";
        break;
    case FirstPage:
        ret = "go_first";
        break;
    case LastPage:
        ret = "go_last";
        break;

    case AddBookmark:
        ret = "bookmark_add";
        break;
    case EditBookmarks:
        ret = "bookmark_edit";
        break;

    case Spelling:
        ret = "tools_spelling";
        break;

    case ShowMenubar:
        ret = "options_show_menubar";
        break;
    case ShowToolbar:
        ret = "options_show_toolbar";
        break;
    case ShowStatusbar:
        ret = "options_show_statusbar";
        break;
    case SaveOptions:
        ret = "options_save_options";
        break;
    case KeyBindings:
        ret = "options_configure_keybinding";
        break;
    case Preferences:
        ret = "options_configure";
        break;
    case ConfigureToolbars:
        ret = "options_configure_toolbars";
        break;

    case Help:
        ret = "help";
        break;
    case HelpContents:
        ret = "help_contents";
        break;
    case WhatsThis:
        ret = "help_whats_this";
        break;
    case ReportBug:
        ret = "help_report_bug";
        break;
    case AboutApp:
        ret = "help_about_app";
        break;
    case AboutKDE:
        ret = "help_about_kde";
        break;

    default:
        ret = "";
    }

    return ret;
}

KAction *openNew(const QObject *recvr, const char *slot,
                             QObject *parent, const char *name )
{
    return new KAction(i18n("&New"), "filenew",
                       KStdAccel::key(KStdAccel::New), recvr, slot, parent,
                       name ? name : stdName(New));
}

KAction *open(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Open..."), "fileopen",
                       KStdAccel::key(KStdAccel::Open), recvr, slot, parent,
                       name ? name : stdName(Open));
}

KRecentFilesAction *openRecent(const QObject *recvr,
                                    const char *slot, QObject *parent,
                                    const char *name )
{
    return new KRecentFilesAction(i18n("Open &Recent"), 0, recvr, slot, parent,
                                  name ? name : stdName(OpenRecent));
}

KAction *save(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Save"), "filesave",
                       KStdAccel::key(KStdAccel::Save), recvr, slot, parent,
                       name ? name : stdName(Save));
}

KAction *saveAs(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Save &As..."), "filesaveas", 0, recvr, slot, parent,
                       name ? name : stdName(SaveAs));
}

KAction *revert(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Re&vert"), "revert", 0, recvr, slot, parent,
                       name ? name : stdName(Revert));
}

KAction *print(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Print..."), "fileprint",
                       KStdAccel::key(KStdAccel::Print), recvr, slot, parent,
                       name ? name : stdName(Print));
}

KAction *printPreview(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Print Previe&w..."),
                       "filequickprint", 0, recvr, slot,
                       parent, name ? name : stdName(PrintPreview));
}

KAction *close(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Close"), "fileclose",
                       KStdAccel::key(KStdAccel::Close), recvr, slot, parent,
                       name ? name : stdName(Close));
}

KAction *mail(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Mail..."), "mail_send", 0,
                       recvr, slot, parent, name ? name : stdName(Mail));
}

KAction *quit(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Quit"), "exit",
                       KStdAccel::key(KStdAccel::Quit), recvr, slot, parent,
                       name ? name : stdName(Quit));
}

KAction *undo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Undo"), "undo",
                       KStdAccel::key(KStdAccel::Undo), recvr, slot, parent,
                       name ? name : stdName(Undo));
}

KAction *redo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Re&do"), "redo",
                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       name ? name : stdName(Redo));
}

KAction *cut(const QObject *recvr, const char *slot,
                                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Cu&t"), "editcut",
                       KStdAccel::key(KStdAccel::Cut), recvr, slot, parent,
                       name ? name : stdName(Cut));
}

KAction *copy(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
	return create( Copy, dynamic_cast<KActionCollection*>(parent), recvr, slot, name );
//    return new KAction(i18n("&Copy"), "editcopy",
//                       KStdAccel::key(KStdAccel::Copy), recvr, slot, parent,
//                       name ? name : stdName(Copy));
}

KAction *paste(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Paste"), "editpaste",
                       KStdAccel::key(KStdAccel::Paste), recvr, slot, parent,
                       name ? name : stdName(Paste));
}

KAction *selectAll(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("Select &All"),
                       KStdAccel::key(KStdAccel::SelectAll), recvr, slot, parent,
                       name ? name : stdName(SelectAll));
}

KAction *deselect(const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
    return create(Deselect, dynamic_cast<KActionCollection*>(parent), recvr, slot, name );
}

KAction *find(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Find..."), "find",
                       KStdAccel::key(KStdAccel::Find), recvr, slot, parent,
                       name ? name : stdName(Find));
}

KAction *findNext(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Next"), "next",
                       KStdAccel::key(KStdAccel::FindNext),
                       recvr, slot, parent,
                       name ? name : stdName(FindNext));
}

KAction *findPrev(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Previous"), "previous",
                       KStdAccel::key(KStdAccel::FindPrev),
                       recvr, slot, parent, name ? name : stdName(FindPrev));
}

KAction *replace(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("&Replace..."), KStdAccel::key(KStdAccel::Replace),
                       recvr, slot, parent,
                                           name ? name : stdName(Replace));
}

KAction *actualSize(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("&Actual Size"), 0, recvr, slot, parent,
                       name ? name : stdName(ActualSize));
}

KAction *fitToPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Fit To Page"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToPage));
}

KAction *fitToWidth(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Width"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToWidth));
}

KAction *fitToHeight(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Height"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToHeight));
}

KAction *zoomIn(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &In"), "viewmag+",
                       KStdAccel::key(KStdAccel::ZoomIn), recvr, slot, parent,
                       name ? name : stdName(ZoomIn));
}


KAction *zoomOut(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &Out"), "viewmag-",
                       KStdAccel::key(KStdAccel::ZoomOut), recvr, slot, parent,
                       name ? name : stdName(ZoomOut));
}

KAction *zoom(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Zoom..."), "viewmag", 0,
                       recvr, slot, parent, name ? name :
                       stdName(Zoom));
}

KAction *redisplay(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Redisplay"), "reload", 0,
                           recvr, slot, parent, name ? name :
                       stdName(Redisplay));
}

KAction *up(const QObject *recvr, const char *slot,
                                                QObject *parent, const char *name )
{
    return new KAction(i18n("&Up"), "up",  KStdAccel::key(KStdAccel::Up), recvr, slot,
                       parent, name ? name : stdName(Up));
}

KAction *back(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("go back", "&Back"), "back", KStdAccel::key(KStdAccel::Back),
                       recvr, slot, parent, name ? name :
                       stdName(Back));
}

KAction *forward(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("go forward", "&Forward"), "forward", KStdAccel::key(KStdAccel::Forward),
                       recvr, slot, parent, name ? name :
                       stdName(Forward));
}

KAction *home(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("beginning (of line)", "&Home"), "gohome",
                                           KStdAccel::key(KStdAccel::Home), recvr, slot, parent,
                                           name ? name : stdName(Home));
}

KAction *prior(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Previous Page"), "previous",
                       KStdAccel::key(KStdAccel::Prior), recvr, slot, parent,
                       name ? name : stdName(Prior));
}

KAction *next(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Next Page"), "next",
                       KStdAccel::key(KStdAccel::Next), recvr, slot, parent,
                       name ? name : stdName(Next));
}

KAction *goTo(const QObject *recvr, const char *slot,
                          QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to..."), 0, recvr, slot, parent,
                       name ? name : stdName(Goto));
}

KAction *gotoPage(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to Page..."), "goto", 0, recvr, slot, parent,
                       name ? name : stdName(GotoPage));
}

KAction *gotoLine(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to Line..."),
                       KStdAccel::key(KStdAccel::GotoLine), recvr, slot, parent,
                       name ? name : stdName(GotoLine));
}

KAction *firstPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&First Page"), "top",
                       0, recvr, slot, parent,
                       name ? name : stdName(FirstPage));
}

KAction *lastPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Last Page"), "bottom",
                       0, recvr, slot, parent,
                       name ? name : stdName(LastPage));
}

KAction *addBookmark(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Add Bookmark"), "bookmark_add",
                       KStdAccel::key(KStdAccel::AddBookmark),
                       recvr, slot, parent,
                                           name ? name : stdName(AddBookmark));
}

KAction *editBookmarks(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Edit Bookmarks"), "bookmark", 0, recvr, slot, parent,
                       name ? name : stdName(EditBookmarks));
}

KAction *spelling(const QObject *recvr, const char *slot,
                                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Spelling..."), "spellcheck",
                             0, recvr, slot, parent,
                             name ? name : stdName(Spelling));
}

KToggleAction *showMenubar(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Menubar"), "showmenu", KStdAccel::key(KStdAccel::ShowMenubar), recvr, slot,
                            parent, name ? name : stdName(ShowMenubar));
    ret->setChecked(true);
    return ret;
}

KToggleAction *showToolbar(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent,
                            name ? name : stdName(ShowToolbar));
    ret->setChecked(true);
    return ret;

}

KToggleAction *showStatusbar(const QObject *recvr, const char *slot,
                                         QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show St&atusbar"), 0, recvr, slot, parent,
                            name ? name : stdName(ShowStatusbar));
    ret->setChecked(true);
    return ret;
}

KAction *saveOptions(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Save Settings"), 0, recvr, slot,
                       parent, name ? name : stdName(SaveOptions));
}

KAction *keyBindings(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Configure S&hortcuts..."), "key_bindings", 0, recvr, slot,
                       parent, name ? name : stdName(KeyBindings));
}

KAction *preferences(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    const KAboutData *aboutData = KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());

    return new KAction(i18n("&Configure %1...").arg(appName), "configure",
                       0, recvr, slot, parent,
                       name ? name : stdName(Preferences));
}

KAction *configureToolbars(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    return new KAction(i18n("Configure Tool&bars..."), "style", 0, recvr,
                       slot, parent, name ? name : stdName(ConfigureToolbars));
}

KAction *help(const QObject *recvr, const char *slot,
                          QObject *parent, const char *name )
{
    return new KAction(i18n("show help", "&Help"), "help",
                       KStdAccel::key(KStdAccel::Help), recvr, slot, parent,
                       name ? name : stdName(Help));
}

KAction *helpContents(const QObject *recvr, const char *slot,
                                                          QObject *parent, const char *name )
{
    const KAboutData *aboutData = KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());
    return new KAction(i18n("%1 &Handbook").arg(appName), "contents",
                       KStdAccel::key(KStdAccel::Help), recvr, slot, parent,
                       name ? name : stdName(HelpContents));
}

KAction *whatsThis(const QObject *recvr, const char *slot,
                                                       QObject *parent, const char *name )
{
    return new KAction(i18n("What's &This?"), "contexthelp",
                       KStdAccel::key(KStdAccel::WhatThis), recvr, slot,
                       parent, name ? name : stdName(WhatsThis));
}

KAction *reportBug(const QObject *recvr, const char *slot,
                                                       QObject *parent, const char *name )
{
    return new KAction(i18n("&Report Bug..."), 0, recvr, slot,
                       parent, name ? name : stdName(ReportBug));
}

KAction *aboutApp(const QObject *recvr, const char *slot,
                                                      QObject *parent, const char *name )
{
    const KAboutData *aboutData = KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());
    return new KAction(i18n("&About %1").arg(appName),
                       kapp->miniIconName(), 0, recvr,
                       slot, parent, name ? name : stdName(AboutApp));
}

KAction *aboutKDE(const QObject *recvr, const char *slot,
                                                      QObject *parent, const char *name )
{
    return new KAction(i18n("About &KDE"), "go", 0, recvr, slot,
                       parent, name ? name : stdName(AboutKDE));
}

};
