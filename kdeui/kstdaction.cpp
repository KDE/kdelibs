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
#include <kaboutdata.h>

#include <kapplication.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>

KStdAction::KStdAction()
{
}

KStdAction::~KStdAction()
{
}

KAction *KStdAction::action(StdAction act_enum, const QObject *recvr,
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

const char* KStdAction::stdName(StdAction act_enum)
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

KAction *KStdAction::openNew(const QObject *recvr, const char *slot,
                             QObject *parent, const char *name )
{
    return new KAction(i18n("&New"), "filenew",
                       KStdAccel::key(KStdAccel::New), recvr, slot, parent,
                       name ? name : stdName(New));
}

KAction *KStdAction::open(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Open..."), "fileopen",
                       KStdAccel::key(KStdAccel::Open), recvr, slot, parent,
                       name ? name : stdName(Open));
}

KRecentFilesAction *KStdAction::openRecent(const QObject *recvr,
                                    const char *slot, QObject *parent,
                                    const char *name )
{
    return new KRecentFilesAction(i18n("Open &Recent"), 0, recvr, slot, parent,
                                  name ? name : stdName(OpenRecent));
}

KAction *KStdAction::save(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Save"), "filesave",
                       KStdAccel::key(KStdAccel::Save), recvr, slot, parent,
                       name ? name : stdName(Save));
}

KAction *KStdAction::saveAs(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Save &As..."), "filesaveas", 0, recvr, slot, parent,
                       name ? name : stdName(SaveAs));
}

KAction *KStdAction::revert(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Re&vert"), "revert", 0, recvr, slot, parent,
                       name ? name : stdName(Revert));
}

KAction *KStdAction::print(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Print..."), "fileprint",
                       KStdAccel::key(KStdAccel::Print), recvr, slot, parent,
                       name ? name : stdName(Print));
}

KAction *KStdAction::printPreview(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Print Previe&w..."),
                       "filequickprint", 0, recvr, slot,
                       parent, name ? name : stdName(PrintPreview));
}

KAction *KStdAction::close(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Close"), "fileclose",
                       KStdAccel::key(KStdAccel::Close), recvr, slot, parent,
                       name ? name : stdName(Close));
}

KAction *KStdAction::mail(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Mail..."), "mail_send", 0,
                       recvr, slot, parent, name ? name : stdName(Mail));
}

KAction *KStdAction::quit(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Quit"), "exit",
                       KStdAccel::key(KStdAccel::Quit), recvr, slot, parent,
                       name ? name : stdName(Quit));
}

KAction *KStdAction::undo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Und&o"), "undo",
                       KStdAccel::key(KStdAccel::Undo), recvr, slot, parent,
                       name ? name : stdName(Undo));
}

KAction *KStdAction::redo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Re&do"), "redo",
                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       name ? name : stdName(Redo));
}

KAction *KStdAction::cut(const QObject *recvr, const char *slot,
                                                 QObject *parent, const char *name )
{
    return new KAction(i18n("C&ut"), "editcut",
                       KStdAccel::key(KStdAccel::Cut), recvr, slot, parent,
                       name ? name : stdName(Cut));
}

KAction *KStdAction::copy(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Copy"), "editcopy",
                       KStdAccel::key(KStdAccel::Copy), recvr, slot, parent,
                       name ? name : stdName(Copy));
}

KAction *KStdAction::paste(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Paste"), "editpaste",
                       KStdAccel::key(KStdAccel::Paste), recvr, slot, parent,
                       name ? name : stdName(Paste));
}

KAction *KStdAction::selectAll(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("Select &All"), 
                       KStdAccel::key(KStdAccel::SelectAll), recvr, slot, parent,
                       name ? name : stdName(SelectAll));
}

KAction *KStdAction::find(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Find..."), "find",
                       KStdAccel::key(KStdAccel::Find), recvr, slot, parent,
                       name ? name : stdName(Find));
}

KAction *KStdAction::findNext(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Next"), "next",
                       KStdAccel::key(KStdAccel::FindNext),
                       recvr, slot, parent,
                       name ? name : stdName(FindNext));
}

KAction *KStdAction::findPrev(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("Find &Previous"), "previous",
                       KStdAccel::key(KStdAccel::FindPrev),
                       recvr, slot, parent, name ? name : stdName(FindPrev));
}

KAction *KStdAction::replace(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("&Replace..."), KStdAccel::key(KStdAccel::Replace),
                       recvr, slot, parent,
                                           name ? name : stdName(Replace));
}

KAction *KStdAction::actualSize(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("&Actual Size"), 0, recvr, slot, parent,
                       name ? name : stdName(ActualSize));
}

KAction *KStdAction::fitToPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Fit To Page"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToPage));
}

KAction *KStdAction::fitToWidth(const QObject *recvr, const char *slot,
                                QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Width"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToWidth));
}

KAction *KStdAction::fitToHeight(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Fit To Page &Height"), 0, recvr, slot, parent,
                       name ? name : stdName(FitToHeight));
}

KAction *KStdAction::zoomIn(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &In"), "viewmag+",
                       KStdAccel::key(KStdAccel::ZoomIn), recvr, slot, parent,
                       name ? name : stdName(ZoomIn));
}


KAction *KStdAction::zoomOut(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("Zoom &Out"), "viewmag-",
                       KStdAccel::key(KStdAccel::ZoomOut), recvr, slot, parent,
                       name ? name : stdName(ZoomOut));
}

KAction *KStdAction::zoom(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Zoom..."), "viewmag", 0,
                       recvr, slot, parent, name ? name :
                       stdName(Zoom));
}

KAction *KStdAction::redisplay(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Redisplay"), "reload", 0,
                           recvr, slot, parent, name ? name :
                       stdName(Redisplay));
}

KAction *KStdAction::up(const QObject *recvr, const char *slot,
                                                QObject *parent, const char *name )
{
    return new KAction(i18n("&Up"), "up",  KStdAccel::key(KStdAccel::Up), recvr, slot,
                       parent, name ? name : stdName(Up));
}

KAction *KStdAction::back(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("go back", "&Back"), "back", KStdAccel::key(KStdAccel::Back),
                       recvr, slot, parent, name ? name :
                       stdName(Back));
}

KAction *KStdAction::forward(const QObject *recvr, const char *slot,
                                                         QObject *parent, const char *name )
{
    return new KAction(i18n("go forward", "&Forward"), "forward", KStdAccel::key(KStdAccel::Forward),
                       recvr, slot, parent, name ? name :
                       stdName(Forward));
}

KAction *KStdAction::home(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("beginning (of line)", "&Home"), "gohome",
                                           KStdAccel::key(KStdAccel::Home), recvr, slot, parent,
                                           name ? name : stdName(Home));
}

KAction *KStdAction::prior(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Previous Page"), "previous",
                       KStdAccel::key(KStdAccel::Prior), recvr, slot, parent,
                       name ? name : stdName(Prior));
}

KAction *KStdAction::next(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Next Page"), "next",
                       KStdAccel::key(KStdAccel::Next), recvr, slot, parent,
                       name ? name : stdName(Next));
}

KAction *KStdAction::goTo(const QObject *recvr, const char *slot,
                          QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to..."), 0, recvr, slot, parent,
                       name ? name : stdName(Goto));
}

KAction *KStdAction::gotoPage(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to Page..."), "goto", 0, recvr, slot, parent,
                       name ? name : stdName(GotoPage));
}

KAction *KStdAction::gotoLine(const QObject *recvr, const char *slot,
                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Go to Line..."),
                       KStdAccel::key(KStdAccel::GotoLine), recvr, slot, parent,
                       name ? name : stdName(GotoLine));
}

KAction *KStdAction::firstPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&First Page"), "top",
                       0, recvr, slot, parent,
                       name ? name : stdName(FirstPage));
}

KAction *KStdAction::lastPage(const QObject *recvr, const char *slot,
                               QObject *parent, const char *name )
{
    return new KAction(i18n("&Last Page"), "bottom",
                       0, recvr, slot, parent,
                       name ? name : stdName(LastPage));
}

KAction *KStdAction::addBookmark(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Add Bookmark"), "bookmark_add",
                       KStdAccel::key(KStdAccel::AddBookmark),
                       recvr, slot, parent,
                                           name ? name : stdName(AddBookmark));
}

KAction *KStdAction::editBookmarks(const QObject *recvr, const char *slot,
                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Edit Bookmarks"), "bookmark", 0, recvr, slot, parent,
                       name ? name : stdName(EditBookmarks));
}

KAction *KStdAction::spelling(const QObject *recvr, const char *slot,
                                              QObject *parent, const char *name )
{
    return new KAction(i18n("&Spelling..."), "spellcheck",
                             0, recvr, slot, parent,
                             name ? name : stdName(Spelling));
}

KToggleAction *KStdAction::showMenubar(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Menubar"), "showmenu", KStdAccel::key(KStdAccel::ShowMenubar), recvr, slot,
                            parent, name ? name : stdName(ShowMenubar));
    ret->setChecked(true);
    return ret;
}

KToggleAction *KStdAction::showToolbar(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent,
                            name ? name : stdName(ShowToolbar));
    ret->setChecked(true);
    return ret;

}

KToggleAction *KStdAction::showStatusbar(const QObject *recvr, const char *slot,
                                         QObject *parent, const char *name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show St&atusbar"), 0, recvr, slot, parent,
                            name ? name : stdName(ShowStatusbar));
    ret->setChecked(true);
    return ret;
}

KAction *KStdAction::saveOptions(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("&Save Settings"), 0, recvr, slot,
                       parent, name ? name : stdName(SaveOptions));
}

KAction *KStdAction::keyBindings(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    return new KAction(i18n("Configure &Key Bindings..."), "key_bindings", 0, recvr, slot,
                       parent, name ? name : stdName(KeyBindings));
}

KAction *KStdAction::preferences(const QObject *recvr, const char *slot,
                                 QObject *parent, const char *name )
{
    const KAboutData *aboutData = KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());

    return new KAction(i18n("&Configure %1...").arg(appName), "configure",
                       0, recvr, slot, parent,
                       name ? name : stdName(Preferences));
}

KAction *KStdAction::configureToolbars(const QObject *recvr, const char *slot,
                                       QObject *parent, const char *name )
{
    return new KAction(i18n("Configure Tool&bars..."), "style", 0, recvr,
                       slot, parent, name ? name : stdName(ConfigureToolbars));
}

KAction *KStdAction::help(const QObject *recvr, const char *slot,
                          QObject *parent, const char *name )
{
    return new KAction(i18n("show help", "&Help"), "help",
                       KStdAccel::key(KStdAccel::Help), recvr, slot, parent,
                       name ? name : stdName(Help));
}

KAction *KStdAction::helpContents(const QObject *recvr, const char *slot,
                                                          QObject *parent, const char *name )
{
    return new KAction(i18n("&Contents"), "contents",
                       KStdAccel::key(KStdAccel::Help), recvr, slot, parent,
                       name ? name : stdName(HelpContents));
}

KAction *KStdAction::whatsThis(const QObject *recvr, const char *slot,
                                                       QObject *parent, const char *name )
{
    return new KAction(i18n("What's &This?"), "contexthelp",
                       KStdAccel::key(KStdAccel::WhatThis), recvr, slot,
                       parent, name ? name : stdName(WhatsThis));
}

KAction *KStdAction::reportBug(const QObject *recvr, const char *slot,
                                                       QObject *parent, const char *name )
{
    return new KAction(i18n("&Report Bug..."), 0, recvr, slot,
                       parent, name ? name : stdName(ReportBug));
}

KAction *KStdAction::aboutApp(const QObject *recvr, const char *slot,
                                                      QObject *parent, const char *name )
{
    const KAboutData *aboutData = KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());
    return new KAction(i18n("&About %1").arg(appName),
                       QIconSet(kapp->miniIcon()), 0, recvr,
                       slot, parent, name ? name : stdName(AboutApp));
}

KAction *KStdAction::aboutKDE(const QObject *recvr, const char *slot,
                                                      QObject *parent, const char *name )
{
    return new KAction(i18n("About &KDE"), "go", 0, recvr, slot,
                       parent, name ? name : stdName(AboutKDE));
}
