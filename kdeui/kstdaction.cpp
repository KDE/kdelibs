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
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kaction.h>

KStdAction::KStdAction()
{
}

KStdAction::~KStdAction()
{
}

QAction *KStdAction::action(StdAction act_enum, const QObject *recvr,
                            const char *slot, QObject *parent, const char *name )
{
    QAction *act;
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
        act = gotoPage(recvr, slot, parent, name);
        break;
    case FirstPage:
        act = firstPage(recvr, slot, parent, name);
        break;
    case LastPage:
        act = lastPage(recvr, slot, parent, name);
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
    case KeyBindings:
        act = keyBindings(recvr, slot, parent, name);
        break;
    case Preferences:
        act = preferences(recvr, slot, parent, name);
        break;

    case Help:
        act = help(recvr, slot, parent, name);
        break;

    default:
        act = 0;
    }

    return act;
}

KAction *KStdAction::openNew(const QObject *recvr, const char *slot,
                             QObject *parent, const char *name )
{
    return new KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
                       KStdAccel::key(KStdAccel::New), recvr, slot, parent,
                       name ? name : "file_new");
}

KAction *KStdAction::open(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Open"), QIconSet(BarIcon("fileopen")),
                       KStdAccel::key(KStdAccel::Open), recvr, slot, parent,
                       name ? name : "file_open");
}

KSelectAction *KStdAction::openRecent(const QObject *recvr, const char *slot,
				      QObject *parent, const char *name )
{
    return new KSelectAction(i18n("Open &Recent"), 0, recvr, slot, parent,
                             name ? name : "file_open_recent");
}

KAction *KStdAction::save(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Save"), QIconSet(BarIcon("filefloppy")),
                       KStdAccel::key(KStdAccel::Save), recvr, slot, parent,
                       name ? name : "file_save");
}

KAction *KStdAction::saveAs(const QObject *recvr, const char *slot,
							QObject *parent, const char *name )
{
    return new KAction(i18n("Save &As..."), 0, recvr, slot, parent,
                       name ? name : "file_save_as");
}

KAction *KStdAction::revert(const QObject *recvr, const char *slot,
							QObject *parent, const char *name )
{
    return new KAction(i18n("Re&vert"), 0, recvr, slot, parent,
                       name ? name : "file_revert");
}

KAction *KStdAction::print(const QObject *recvr, const char *slot,
						   QObject *parent, const char *name )
{
    return new KAction(i18n("&Print..."), QIconSet(BarIcon("fileprint")),
                       KStdAccel::key(KStdAccel::Print), recvr, slot, parent,
                       name ? name : "file_print");
}

KAction *KStdAction::printPreview(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Print Previe&w..."),
                       QIconSet(BarIcon("print_preview")), 0, recvr, slot,
                       parent, name ? name : "file_print_preview");
}

KAction *KStdAction::close(const QObject *recvr, const char *slot,
						   QObject *parent, const char *name )
{
    return new KAction(i18n("&Close"), QIconSet(BarIcon("close")),
                       KStdAccel::key(KStdAccel::Close), recvr, slot, parent,
                       name ? name : "file_close");
}

KAction *KStdAction::quit(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Quit"), QIconSet(BarIcon("exit")),
                       KStdAccel::key(KStdAccel::Quit), recvr, slot, parent,
                       name ? name : "file_quit");
}

KAction *KStdAction::undo(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("Und&o"), QIconSet(BarIcon("undo")),
                       KStdAccel::key(KStdAccel::Undo), recvr, slot, parent,
                       name ? name : "edit_undo");
}

KAction *KStdAction::redo(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("Re&do"), QIconSet(BarIcon("redo")),
                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       name ? name : "edit_redo");
}

KAction *KStdAction::cut(const QObject *recvr, const char *slot,
						 QObject *parent, const char *name )
{
    return new KAction(i18n("C&ut"), QIconSet(BarIcon("editcut")),
                       KStdAccel::key(KStdAccel::Cut), recvr, slot, parent,
                       name ? name : "edit_cut");
}

KAction *KStdAction::copy(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Copy"), QIconSet(BarIcon("editcopy")),
                       KStdAccel::key(KStdAccel::Copy), recvr, slot, parent,
                       name ? name : "edit_copy");
}

KAction *KStdAction::paste(const QObject *recvr, const char *slot,
						   QObject *parent, const char *name )
{
    return new KAction(i18n("&Paste"), QIconSet(BarIcon("editpaste")),
                       KStdAccel::key(KStdAccel::Paste), recvr, slot, parent,
                       name ? name : "edit_paste");
}

KAction *KStdAction::selectAll(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("Select &All"), 0, recvr, slot, parent,
                       name ? name : "edit_select_all");
}

KAction *KStdAction::find(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Find"), QIconSet(BarIcon("find")),
                       KStdAccel::key(KStdAccel::Find), recvr, slot, parent,
                       name ? name : "edit_find");
}

KAction *KStdAction::findNext(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Next"), KStdAccel::key(KStdAccel::FindNext),
                       recvr, slot, parent,
                       name ? name : "edit_file_next");
}

KAction *KStdAction::findPrev(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Previous"), KStdAccel::key(KStdAccel::FindPrev),
                       recvr, slot, parent,
                       name ? name : "edit_find_last");
}

KAction *KStdAction::replace(const QObject *recvr, const char *slot,
							 QObject *parent, const char *name )
{
    return new KAction(i18n("&Replace"), KStdAccel::key(KStdAccel::Replace),
                       recvr, slot, parent,
					   name ? name : "edit_replace");
}

KAction *KStdAction::actualSize(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("&Actual Size"), 0, recvr, slot, parent,
                       name ? name : "view_actual_size");
}

KAction *KStdAction::fitToPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Fit To Page"), 0, recvr, slot, parent,
                       name ? name : "view_fit_to_page");
}

KAction *KStdAction::fitToWidth(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Width"), 0, recvr, slot, parent,
                       name ? name : "view_fit_to_width");
}

KAction *KStdAction::fitToHeight(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Height"), 0, recvr, slot, parent,
                       name ? name : "view_fit_to_height");
}

KAction *KStdAction::zoomIn(const QObject *recvr, const char *slot,
							QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &In"), QIconSet(BarIcon("viewmag+")),
                       KStdAccel::key(KStdAccel::ZoomIn), recvr, slot, parent,
                       name ? name : "view_zoom_in");
}


KAction *KStdAction::zoomOut(const QObject *recvr, const char *slot,
							 QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &Out"), QIconSet(BarIcon("viewmag-")),
                       KStdAccel::key(KStdAccel::ZoomOut), recvr, slot, parent,
                       name ? name : "view_zoom_out");
}

KAction *KStdAction::zoom(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Zoom..."), QIconSet(BarIcon("viewzoom")), 0,
                       recvr, slot, parent, name ? name : "view_zoom");
}

KAction *KStdAction::redisplay(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Redisplay"), QIconSet(BarIcon("reload")), 0,
	                   recvr, slot, parent, name ? name : "view_redisplay");
}

KAction *KStdAction::up(const QObject *recvr, const char *slot,
						QObject *parent, const char *name )
{
    return new KAction(i18n("&Up"), QIconSet(BarIcon("up")), 0, recvr, slot,
                       parent, name ? name : "go_up");
}

KAction *KStdAction::back(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Back"), QIconSet(BarIcon("back")), 0,
                       recvr, slot, parent, name ? name : "go_back");
}

KAction *KStdAction::forward(const QObject *recvr, const char *slot,
							 QObject *parent, const char *name )
{
    return new KAction(i18n("&Forward"), QIconSet(BarIcon("forward")), 0,
                       recvr, slot, parent, name ? name : "go_forward");
}

KAction *KStdAction::home(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Home"), QIconSet(BarIcon("home")),
					   KStdAccel::key(KStdAccel::Home), recvr, slot, parent,
					   name ? name : "go_home");
}

KAction *KStdAction::prior(const QObject *recvr, const char *slot,
						   QObject *parent, const char *name )
{
    return new KAction(i18n("&Previous Page"), QIconSet(BarIcon("prev")),
                       KStdAccel::key(KStdAccel::Prior), recvr, slot, parent,
                       name ? name : "go_previous");
}

KAction *KStdAction::next(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Next Page"), QIconSet(BarIcon("next")),
                       KStdAccel::key(KStdAccel::Next), recvr, slot, parent,
                       name ? name : "go_next");
}

KAction *KStdAction::gotoPage(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to Page..."), 0, recvr, slot, parent,
                       name ? name : "go_goto");
}

KAction *KStdAction::firstPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&First Page"), 0, recvr, slot, parent,
                       name ? name : "go_first");
}

KAction *KStdAction::lastPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Last Page"), 0, recvr, slot, parent,
                       name ? name : "go_last");
}

KAction *KStdAction::addBookmark(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Add Bookmark"), KStdAccel::key(KStdAccel::AddBookmark),
                       recvr, slot, parent,
					   name ? name : "bookmark_add");
}

KAction *KStdAction::editBookmarks(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Edit Bookmarks..."), 0, recvr, slot, parent,
                       name ? name : "bookmark_edit");
}

KAction *KStdAction::spelling(const QObject *recvr, const char *slot,
				              QObject *parent, const char *name )
{
    return new KAction(i18n("&Spelling..."), QIconSet(BarIcon("spellcheck")),
                             0, recvr, slot, parent,
                             name ? name : "tools_spelling");
}

KToggleAction *KStdAction::showMenubar(const QObject *recvr, const char *slot,
				       QObject *parent, const char *name )
{
    return new KToggleAction(i18n("Show &Menubar"), 0, recvr, slot, parent,
                             name ? name : "options_show_menubar");
}

KToggleAction *KStdAction::showToolbar(const QObject *recvr, const char *slot,
				       QObject *parent, const char *name )
{
    return new KToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent,
                             name ? name : "options_show_toolbar");
}

KToggleAction *KStdAction::showStatusbar(const QObject *recvr, const char *slot,
					 QObject *parent, const char *name )
{
    return new KToggleAction(i18n("Show &Statusbar"), 0, recvr, slot, parent,
                             name ? name : "options_show_statusbar");
}

KAction *KStdAction::keyBindings(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Configure &Key Bindings..."), 0, recvr, slot,
                       parent, name ? name : "options_configure_keybinding");
}

KAction *KStdAction::preferences(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Preferences..."), QIconSet(BarIcon("options")),
                       0, recvr, slot, parent,
                       name ? name : "options_configure");
}

KAction *KStdAction::help(const QObject *recvr, const char *slot,
						  QObject *parent, const char *name )
{
    return new KAction(i18n("&Help"), QIconSet(BarIcon("help")),
                       KStdAccel::key(KStdAccel::Help), recvr, slot, parent,
                       name ? name : "help");
}
