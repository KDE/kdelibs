/* This file is part of the KDE libraries
   Copyright (C) 1999 Kurt Granroth <granroth@kde.org>

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
                            const char *slot, QObject *parent)
{
    QAction *act;
    switch (act_enum)
    {
    case New:
        act = openNew(recvr, slot, parent);
        break;
    case Open:
        act = open(recvr, slot, parent);
        break;
    case OpenRecent:
        act = openRecent(recvr, slot, parent);
        break;
    case Save:
        act = save(recvr, slot, parent);
        break;
    case SaveAs:
        act = saveAs(recvr, slot, parent);
        break;
    case Revert:
        act = revert(recvr, slot, parent);
        break;
    case Close:
        act = close(recvr, slot, parent);
        break;
    case Print:
        act = print(recvr, slot, parent);
        break;
    case PrintPreview:
        act = printPreview(recvr, slot, parent);
        break;
    case Quit:
        act = quit(recvr, slot, parent);
        break;

    case Undo:
        act = undo(recvr, slot, parent);
        break;
    case Redo:
        act = redo(recvr, slot, parent);
        break;
    case Cut:
        act = cut(recvr, slot, parent);
        break;
    case Copy:
        act = copy(recvr, slot, parent);
        break;
    case Paste:
        act = paste(recvr, slot, parent);
        break;
    case SelectAll:
        act = selectAll(recvr, slot, parent);
        break;
    case Find:
        act = find(recvr, slot, parent);
        break;
    case FindNext:
        act = findNext(recvr, slot, parent);
        break;
    case FindPrev:
        act = findPrev(recvr, slot, parent);
        break;
    case Replace:
        act = replace(recvr, slot, parent);
        break;

    case ActualSize:
        act = actualSize(recvr, slot, parent);
        break;
    case FitToPage:
        act = fitToPage(recvr, slot, parent);
        break;
    case FitToWidth:
        act = fitToWidth(recvr, slot, parent);
        break;
    case FitToHeight:
        act = fitToHeight(recvr, slot, parent);
        break;
    case ZoomIn:
        act = zoomIn(recvr, slot, parent);
        break;
    case ZoomOut:
        act = zoomOut(recvr, slot, parent);
        break;
    case Zoom:
        act = zoom(recvr, slot, parent);
        break;
    case Redisplay:
        act = redisplay(recvr, slot, parent);
        break;

    case Up:
        act = up(recvr, slot, parent);
        break;
    case Back:
        act = back(recvr, slot, parent);
        break;
    case Forward:
        act = forward(recvr, slot, parent);
        break;
    case Home:
        act = home(recvr, slot, parent);
        break;
    case Prior:
        act = prior(recvr, slot, parent);
        break;
    case Next:
        act = next(recvr, slot, parent);
        break;
    case Goto:
        act = gotoPage(recvr, slot, parent);
        break;
    case FirstPage:
        act = firstPage(recvr, slot, parent);
        break;
    case LastPage:
        act = lastPage(recvr, slot, parent);
        break;

    case ShowMenubar:
        act = showMenubar(recvr, slot, parent);
        break;
    case ShowToolbar:
        act = showToolbar(recvr, slot, parent);
        break;
    case ShowStatusbar:
        act = showStatusbar(recvr, slot, parent);
        break;
    case KeyBindings:
        act = keyBindings(recvr, slot, parent);
        break;
    case Preferences:
        act = preferences(recvr, slot, parent);
        break;

    case Help:
        act = help(recvr, slot, parent);
        break;

    default:
        act = 0;
    }

    return act;
}

KAction *KStdAction::openNew(const QObject *recvr, const char *slot,
                             QObject *parent)
{
    return new KAction(i18n("&New"), QIconSet(BarIcon("filenew")),
                       KStdAccel::openNew(), recvr, slot, parent);
}

KAction *KStdAction::open(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Open"), QIconSet(BarIcon("fileopen")),
                       KStdAccel::open(), recvr, slot, parent);
}

KSelectAction *KStdAction::openRecent(const QObject *recvr, const char *slot,
				      QObject *parent)
{
    return new KSelectAction(i18n("Open &Recent"), 0, recvr, slot, parent);
}

KAction *KStdAction::save(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Save"), QIconSet(BarIcon("filefloppy")),
                       KStdAccel::save(), recvr, slot, parent);
}

KAction *KStdAction::saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Save &As..."), 0, recvr, slot, parent);
}

KAction *KStdAction::revert(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Re&vert"), 0, recvr, slot, parent);
}

KAction *KStdAction::print(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Print..."), QIconSet(BarIcon("fileprint")),
                       KStdAccel::print(), recvr, slot, parent);
}

KAction *KStdAction::printPreview(const QObject *recvr, const char *slot,
                                  QObject *parent)
{
    return new KAction(i18n("Print Previe&w..."),
                       QIconSet(BarIcon("print_preview")), 0, recvr, slot,
                       parent);
}

KAction *KStdAction::close(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Close"), QIconSet(BarIcon("close")),
                       KStdAccel::close(), recvr, slot, parent);
}

KAction *KStdAction::quit(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Quit"), QIconSet(BarIcon("exit")),
                       KStdAccel::quit(), recvr, slot, parent);
}

KAction *KStdAction::undo(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Und&o"), QIconSet(BarIcon("undo")),
                       KStdAccel::undo(), recvr, slot, parent);
}

KAction *KStdAction::redo(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Re&do"), QIconSet(BarIcon("redo")),
                       KStdAccel::redo(), recvr, slot, parent);
}

KAction *KStdAction::cut(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("C&ut"), QIconSet(BarIcon("editcut")),
                       KStdAccel::cut(), recvr, slot, parent);
}

KAction *KStdAction::copy(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Copy"), QIconSet(BarIcon("editcopy")),
                       KStdAccel::copy(), recvr, slot, parent);
}

KAction *KStdAction::paste(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Paste"), QIconSet(BarIcon("editpaste")),
                       KStdAccel::paste(), recvr, slot, parent);
}

KAction *KStdAction::selectAll(const QObject *recvr, const char *slot,
                               QObject *parent)
{
    return new KAction(i18n("Select &All"), 0, recvr, slot, parent);
}

KAction *KStdAction::find(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Find"), QIconSet(BarIcon("find")),
                       KStdAccel::find(), recvr, slot, parent);
}

KAction *KStdAction::findNext(const QObject *recvr, const char *slot,
                              QObject *parent)
{
    return new KAction(i18n("Find &Next"), KStdAccel::findNext(),
                       recvr, slot, parent);
}

KAction *KStdAction::findPrev(const QObject *recvr, const char *slot,
                              QObject *parent)
{
    return new KAction(i18n("Find &Last"), KStdAccel::findPrev(),
                       recvr, slot, parent);
}

KAction *KStdAction::replace(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Replace"), KStdAccel::replace(),
                       recvr, slot, parent);
}

KAction *KStdAction::actualSize(const QObject *recvr, const char *slot,
                                QObject *parent)
{
    return new KAction(i18n("&Actual Size"), 0, recvr, slot, parent);
}

KAction *KStdAction::fitToPage(const QObject *recvr, const char *slot,
                               QObject *parent)
{
    return new KAction(i18n("&Fit To Page"), 0, recvr, slot, parent);
}

KAction *KStdAction::fitToWidth(const QObject *recvr, const char *slot,
                                QObject *parent)
{
    return new KAction(i18n("Fit To Page &Width"), 0, recvr, slot, parent);
}

KAction *KStdAction::fitToHeight(const QObject *recvr, const char *slot,
                                 QObject *parent)
{
    return new KAction(i18n("Fit To Page &Height"), 0, recvr, slot, parent);
}

KAction *KStdAction::zoomIn(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Zoom &In"), QIconSet(BarIcon("viewmag+")),
                       KStdAccel::zoomIn(), recvr, slot, parent);
}


KAction *KStdAction::zoomOut(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("Zoom &Out"), QIconSet(BarIcon("viewmag-")),
                       KStdAccel::zoomOut(), recvr, slot, parent);
}

KAction *KStdAction::zoom(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Zoom..."), QIconSet(BarIcon("viewzoom")), 0,
                       recvr, slot, parent);
}

KAction *KStdAction::redisplay(const QObject *recvr, const char *slot,
                               QObject *parent)
{
    return new KAction(i18n("&Redisplay"), 0, recvr, slot, parent);
}

KAction *KStdAction::up(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Up"), QIconSet(BarIcon("up")), 0, recvr, slot,
                       parent);
}

KAction *KStdAction::back(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Back"), QIconSet(BarIcon("back")), 0,
                       recvr, slot, parent);
}

KAction *KStdAction::forward(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Forward"), QIconSet(BarIcon("forward")), 0,
                       recvr, slot, parent);
}

KAction *KStdAction::home(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Home"), QIconSet(BarIcon("home")), 0,
                       recvr, slot, parent);
}

KAction *KStdAction::prior(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Previous Page"), QIconSet(BarIcon("prev")),
                       KStdAccel::prior(), recvr, slot, parent);
}

KAction *KStdAction::next(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Next Page"), QIconSet(BarIcon("next")),
                       KStdAccel::next(), recvr, slot, parent);
}

KAction *KStdAction::gotoPage(const QObject *recvr, const char *slot,
                              QObject *parent)
{
    return new KAction(i18n("&Go to Page..."), 0, recvr, slot, parent);
}

KAction *KStdAction::firstPage(const QObject *recvr, const char *slot,
                               QObject *parent)
{
    return new KAction(i18n("&First Page"), 0, recvr, slot, parent);
}

KAction *KStdAction::lastPage(const QObject *recvr, const char *slot,
                               QObject *parent)
{
    return new KAction(i18n("&Last Page"), 0, recvr, slot, parent);
}

KAction *KStdAction::addBookmark(const QObject *recvr, const char *slot,
                                 QObject *parent)
{
    return new KAction(i18n("&Add Bookmark"), KStdAccel::addBookmark(),
                       recvr, slot, parent);
}

KAction *KStdAction::editBookmarks(const QObject *recvr, const char *slot,
                                  QObject *parent)
{
    return new KAction(i18n("&Edit Bookmarks..."), 0, recvr, slot, parent);
}

KToggleAction *KStdAction::showMenubar(const QObject *recvr, const char *slot,
				       QObject *parent)
{
    return new KToggleAction(i18n("Show &Menubar"), 0, recvr, slot, parent);
}

KToggleAction *KStdAction::showToolbar(const QObject *recvr, const char *slot,
				       QObject *parent)
{
    return new KToggleAction(i18n("Show &Toolbar"), 0, recvr, slot, parent);
}

KToggleAction *KStdAction::showStatusbar(const QObject *recvr, const char *slot,
					 QObject *parent)
{
    return new KToggleAction(i18n("Show &Statusbar"), 0, recvr, slot, parent);
}

KAction *KStdAction::keyBindings(const QObject *recvr, const char *slot,
                                 QObject *parent)
{
    return new KAction(i18n("Configure &Key Bindings..."), 0, recvr, slot,
                       parent);
}

KAction *KStdAction::preferences(const QObject *recvr, const char *slot,
                                 QObject *parent)
{
    return new KAction(i18n("&Preferences..."), QIconSet(BarIcon("options")),
                       0, recvr, slot, parent);
}

KAction *KStdAction::help(const QObject *recvr, const char *slot, QObject *parent)
{
    return new KAction(i18n("&Help"), QIconSet(BarIcon("help")),
                       KStdAccel::help(), recvr, slot, parent);
}
