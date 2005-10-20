#ifndef _KSTDACTION_PRIVATE_H_
#define _KSTDACTION_PRIVATE_H_

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

static const KStdActionInfo g_rgActionInfo[] =
{
	{ New,           KStdAccel::New, "file_new", I18N_NOOP("&New"), 0, "filenew" },
	{ Open,          KStdAccel::Open, "file_open", I18N_NOOP("&Open..."), 0, "fileopen" },
	{ OpenRecent,    KStdAccel::AccelNone, "file_open_recent", I18N_NOOP("Open &Recent"), 0, "fileopen" },
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
	{ PasteText,     KStdAccel::Paste, "edit_paste", I18N_NOOP("&Paste"), 0, "editpaste" },
	{ Clear,         KStdAccel::AccelNone, "edit_clear", I18N_NOOP("C&lear"), 0, "editclear" },
	{ SelectAll,     KStdAccel::SelectAll, "edit_select_all", I18N_NOOP("Select &All"), 0, 0 },
	{ Deselect,      KStdAccel::Deselect, "edit_deselect", I18N_NOOP("Dese&lect"), 0, 0 },
	{ Find,          KStdAccel::Find, "edit_find", I18N_NOOP("&Find..."), 0, "find" },
	{ FindNext,      KStdAccel::FindNext, "edit_find_next", I18N_NOOP("Find &Next"), 0, "next" },
	// FIXME: rename edit_find_last to edit_find_prev for KDE 4
	{ FindPrev,      KStdAccel::FindPrev, "edit_find_last", I18N_NOOP("Find Pre&vious"), 0, "previous" },
	{ Replace,       KStdAccel::Replace, "edit_replace", I18N_NOOP("&Replace..."), 0, 0 },

	{ ActualSize,    KStdAccel::AccelNone, "view_actual_size", I18N_NOOP("&Actual Size"), 0, "viewmag1" },
	{ FitToPage,     KStdAccel::AccelNone, "view_fit_to_page", I18N_NOOP("&Fit to Page"), 0, "view_fit_window" },
	{ FitToWidth,    KStdAccel::AccelNone, "view_fit_to_width", I18N_NOOP("Fit to Page &Width"), 0, "view_fit_width" },
	{ FitToHeight,   KStdAccel::AccelNone, "view_fit_to_height", I18N_NOOP("Fit to Page &Height"), 0, "view_fit_height" },
	{ ZoomIn,        KStdAccel::ZoomIn, "view_zoom_in", I18N_NOOP("Zoom &In"), 0, "viewmag+" },
	{ ZoomOut,       KStdAccel::ZoomOut, "view_zoom_out", I18N_NOOP("Zoom &Out"), 0, "viewmag-" },
	{ Zoom,          KStdAccel::AccelNone, "view_zoom", I18N_NOOP("&Zoom..."), 0, "viewmag" },
        // KDE4: give Redisplay the shortcut KStdAccel::AccelReload
	{ Redisplay,     KStdAccel::AccelNone, "view_redisplay", I18N_NOOP("&Redisplay"), 0, "reload" },

	{ Up,            KStdAccel::Up, "go_up", I18N_NOOP("&Up"), 0, "up" },
	// The following three have special i18n() needs for sLabel
	{ Back,          KStdAccel::Back, "go_back", 0, 0, "back" },
	{ Forward,       KStdAccel::Forward, "go_forward", 0, 0, "forward" },
	{ Home,          KStdAccel::Home, "go_home", 0, 0, "gohome" },
	{ Prior,         KStdAccel::Prior, "go_previous", I18N_NOOP("&Previous Page"), 0, "previous" },
	{ Next,          KStdAccel::Next, "go_next", I18N_NOOP("&Next Page"), 0, "next" },
	{ Goto,          KStdAccel::AccelNone, "go_goto", I18N_NOOP("&Go To..."), 0, 0 },
	{ GotoPage,      KStdAccel::AccelNone, "go_goto_page", I18N_NOOP("&Go to Page..."), 0, "goto" },
	{ GotoLine,      KStdAccel::GotoLine, "go_goto_line", I18N_NOOP("&Go to Line..."), 0, 0 },
	{ FirstPage,     KStdAccel::Home, "go_first", I18N_NOOP("&First Page"), 0, "start" },
	{ LastPage,      KStdAccel::End, "go_last", I18N_NOOP("&Last Page"), 0, "finish" },

	{ AddBookmark,   KStdAccel::AddBookmark, "bookmark_add", I18N_NOOP("&Add Bookmark"), 0, "bookmark_add" },
	{ EditBookmarks, KStdAccel::AccelNone, "bookmark_edit", I18N_NOOP("&Edit Bookmarks"), 0, "bookmark" },

	{ Spelling,      KStdAccel::AccelNone, "tools_spelling", I18N_NOOP("&Spelling..."), 0, "spellcheck" },

	{ ShowMenubar,   KStdAccel::ShowMenubar, "options_show_menubar", I18N_NOOP("Show &Menubar"), 0, "showmenu" },
	{ ShowToolbar,   KStdAccel::AccelNone, "options_show_toolbar", I18N_NOOP("Show &Toolbar"), 0, 0 },
	{ ShowStatusbar, KStdAccel::AccelNone, "options_show_statusbar", I18N_NOOP("Show St&atusbar"), 0, 0 },
	{ FullScreen,    KStdAccel::FullScreen, "fullscreen", I18N_NOOP("F&ull Screen Mode"), 0, "window_fullscreen" },
	{ SaveOptions,   KStdAccel::AccelNone, "options_save_options", I18N_NOOP("&Save Settings"), 0, 0 },
	{ KeyBindings,   KStdAccel::AccelNone, "options_configure_keybinding", I18N_NOOP("Configure S&hortcuts..."), 0,"configure_shortcuts" },
	{ Preferences,   KStdAccel::AccelNone, "options_configure", I18N_NOOP("&Configure %1..."), 0, "configure" },
	{ ConfigureToolbars, KStdAccel::AccelNone, "options_configure_toolbars", I18N_NOOP("Configure Tool&bars..."), 0,"configure_toolbars" },
	{ ConfigureNotifications, KStdAccel::AccelNone, "options_configure_notifications", I18N_NOOP("Configure &Notifications..."), 0, "knotify" },

	// the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
	// shortcut
	{ Help,          KStdAccel::Help, "help", 0, 0, "help" },
	{ HelpContents,  KStdAccel::Help, "help_contents", I18N_NOOP("%1 &Handbook"), 0, "contents" },
	{ WhatsThis,     KStdAccel::WhatsThis, "help_whats_this", I18N_NOOP("What's &This?"), 0, "contexthelp" },
	{ TipofDay,      KStdAccel::AccelNone, "help_show_tip", I18N_NOOP("Tip of the &Day"), 0, "idea" },
	{ ReportBug,     KStdAccel::AccelNone, "help_report_bug", I18N_NOOP("&Report Bug..."), 0, 0 },
	{ AboutApp,      KStdAccel::AccelNone, "help_about_app", I18N_NOOP("&About %1"), 0, 0 },
	{ AboutKDE,      KStdAccel::AccelNone, "help_about_kde", I18N_NOOP("About &KDE"), 0,"about_kde" },
	{ ActionNone, KStdAccel::AccelNone, 0, 0, 0, 0 }
};

inline const KStdActionInfo* infoPtr( StdAction id )
{
	for( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ ) {
		if( g_rgActionInfo[i].id == id )
			return &g_rgActionInfo[i];
	}
	return 0;
}

static inline QStringList internal_stdNames()
{
    QStringList result;

    for( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ )
        if (g_rgActionInfo[i].psLabel)
            result.append(i18n(g_rgActionInfo[i].psLabel));
    return result;
}

}

#endif
