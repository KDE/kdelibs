/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)
    Copyright (C) 2000 Nicolas Hadacek (haadcek@kde.org)

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

#include "kstdaccel.h"

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kaccel.h>

uint KStdAccel::key(StdAccel id)
{
    KConfigGroupSaver saver(KGlobal::config(), "Keys");
    QString a = action(id);
    if (!saver.config()->hasKey(a))
       return defaultKey(id);
    
    QString s = saver.config()->readEntry(a);
    return KAccel::stringToKey(s);
}

bool KStdAccel::isEqual(QKeyEvent* ev, int skey)
{
    int key = ev->key();
    switch ( ev->state() )
    {
        case Qt::AltButton:
            key += Qt::ALT;
            break;
        case Qt::ControlButton:
            key += Qt::CTRL;
            break;
        case Qt::ShiftButton:
            key +=  Qt::SHIFT;
            break;
        case (Qt::AltButton | Qt::ControlButton):
            key += Qt::ALT + Qt::CTRL;
            break;
        case (Qt::AltButton | Qt::ShiftButton):
            key += Qt::ALT + Qt::SHIFT;
            break;
        case (Qt::ControlButton | Qt::ShiftButton):
            key += Qt::CTRL + Qt::SHIFT;
            break;
        default:
            break;
    }
    return (key == skey);
}

QString KStdAccel::action(StdAccel id)
{
    switch (id) {
     case Open:             return "Open";
     case New:              return "New";
     case Close:            return "Close";
     case Save:             return "Save";
     case Print:            return "Print";
     case Quit:             return "Quit";
     case Cut:              return "Cut";
     case Copy:             return "Copy";
     case Paste:            return "Paste";
     case Undo:             return "Undo";
     case Redo:             return "Redo";
     case Find:             return "Find";
     case FindNext:         return "FindNext";
     case FindPrev:         return "FindPrev";
     case Replace:          return "Replace";
     case ZoomIn:           return "ZoomIn";
     case ZoomOut:          return "ZoomOut";
     case Insert:           return "Insert";
     case Home:             return "Home";
     case End:              return "End";
     case Prior:            return "Prior";
     case Next:             return "Next";
     case GotoLine:         return "GotoLine";
     case AddBookmark:      return "AddBookmark";
     case Help:             return "Help";
     case TextCompletion:   return "TextCompletion";
     case PrevCompletion:   return "PrevCompletion";
     case NextCompletion:   return "NextCompletion";
     case RotateUp:         return "RotateUp";
     case RotateDown:       return "RotateDown";
     case PopupMenuContext: return "PopupMenuContext";
     case WhatThis:         return "WhatThis";
     case Reload:           return "Reload";
     case SelectAll:        return "SelectAll";
     case Up:               return "Up";
     case Back:             return "Back";
     case Forward:          return "Forward";
	case ShowMenubar:      return "ShowMenubar";
     case NB_STD_ACCELS:    return QString::null;
    }
    return QString::null;
}

uint KStdAccel::defaultKey(StdAccel id)
{
    switch (id) {
     case Open:             return Qt::CTRL + Qt::Key_O;
     case New:              return Qt::CTRL + Qt::Key_N;
     case Close:            return Qt::CTRL + Qt::Key_W;
     case Save:             return Qt::CTRL + Qt::Key_S;
     case Print:            return Qt::CTRL + Qt::Key_P;
     case Quit:             return Qt::CTRL + Qt::Key_Q;
     case Cut:              return Qt::CTRL + Qt::Key_X;
     case Copy:             return Qt::CTRL + Qt::Key_C;
     case Paste:            return Qt::CTRL + Qt::Key_V;
     case Undo:             return Qt::CTRL + Qt::Key_Z;
     case Redo:             return Qt::SHIFT + Qt::CTRL + Qt::Key_Z;
     case Find:             return Qt::CTRL + Qt::Key_F;
     case FindNext:         return Qt::Key_F3;
     case FindPrev:         return Qt::SHIFT + Qt::Key_F3;
     case Replace:          return Qt::CTRL + Qt::Key_R;
     case ZoomIn:           return Qt::CTRL + Qt::Key_Plus;
     case ZoomOut:          return Qt::CTRL + Qt::Key_Minus;
     case Insert:           return Qt::CTRL + Qt::Key_Insert;
     case Home:             return Qt::CTRL + Qt::Key_Home;
     case End:              return Qt::CTRL + Qt::Key_End;
     case Prior:            return Qt::Key_Prior;
     case Next:             return Qt::Key_Next;
     case GotoLine:         return Qt::CTRL + Qt::Key_G;
     case AddBookmark:      return Qt::CTRL + Qt::Key_B;
     case Help:             return Qt::Key_F1;
     case TextCompletion:   return Qt::CTRL + Qt::Key_E;
     case PrevCompletion:   return Qt::CTRL + Qt::Key_Up;
     case NextCompletion:   return Qt::CTRL + Qt::Key_Down;
     case RotateUp:         return Qt::Key_Up;
     case RotateDown:       return Qt::Key_Down;
     case PopupMenuContext: return Qt::Key_Menu;
     case WhatThis:         return Qt::SHIFT + Qt::Key_F1;
     case Reload:           return Qt::Key_F5;
     case SelectAll:        return Qt::CTRL + Qt::Key_A;
     case Up:               return Qt::ALT + Qt::Key_Up;
     case Back:             return Qt::ALT + Qt::Key_Left;
     case Forward:          return Qt::ALT + Qt::Key_Right;
	case ShowMenubar:      return Qt::CTRL + Qt::Key_M;
     case NB_STD_ACCELS:    return 0;
    }
    
    return 0;
}

QString KStdAccel::description(StdAccel id)
{
    switch (id) {
     case Open:             return i18n("Open");
     case New:              return i18n("New");
     case Close:            return i18n("Close");
     case Save:             return i18n("Save");
     case Print:            return i18n("Print");
     case Quit:             return i18n("Quit");
     case Cut:              return i18n("Cut");
     case Copy:             return i18n("Copy");
     case Paste:            return i18n("Paste");
     case Undo:             return i18n("Undo");
     case Redo:             return i18n("Redo");
     case Find:             return i18n("Find");
     case FindNext:         return i18n("Find Next");
     case FindPrev:         return i18n("Find Prev");
     case Replace:          return i18n("Replace");
     case ZoomIn:           return i18n("Zoom In");
     case ZoomOut:          return i18n("Zoom Out");
     case Insert:           return i18n("Insert");
     case Home:             return i18n("Home");
     case End:              return i18n("End");
     case Prior:            return i18n("Prior");
     case Next:             return i18n("Next");
     case GotoLine:         return i18n("Go to Line");
     case AddBookmark:      return i18n("Add Bookmark");
     case Help:             return i18n("Help");
     case TextCompletion:   return i18n("Text Completion");
     case PrevCompletion:   return i18n("Previous Completion Match");
     case NextCompletion:   return i18n("Next Completion Match");
     case RotateUp:         return i18n("Previous Item in List");
     case RotateDown:       return i18n("Next Item in List");
     case PopupMenuContext: return i18n("Popup Menu Context");
     case WhatThis:         return i18n("What's This");
     case Reload:           return i18n("Reload");
     case SelectAll:        return i18n("Select All");
     case Up:               return i18n("Up");
     case Back:             return i18n("Back");
     case Forward:          return i18n("Forward");
	case ShowMenubar:      return i18n("Show Menu Bar");
     case NB_STD_ACCELS:    return QString::null;
    }
    return QString::null;
}

uint KStdAccel::open()
{
  return key(Open);
}

uint KStdAccel::openNew()
{
  return key(New);
}

uint KStdAccel::close()
{
  return key(Close);
}

uint KStdAccel::save()
{
  return key(Save);
}

uint KStdAccel::print()
{
  return key(Print);
}

uint KStdAccel::quit()
{
  return key(Quit);
}

uint KStdAccel::cut()
{
  return key(Cut);
}

uint KStdAccel::copy()
{
  return key(Copy);
}

uint KStdAccel::paste()
{
  return key(Paste);
}

uint KStdAccel::undo()
{
  return key(Undo);
}

uint KStdAccel::redo()
{
  return key(Redo);
}

uint KStdAccel::find()
{
  return key(Find);
}

uint KStdAccel::findNext()
{
  return key(FindNext);
}

uint KStdAccel::findPrev()
{
  return key(FindPrev);
}

uint KStdAccel::replace()
{
  return key(Replace);
}


uint KStdAccel::zoomIn()
{
  return key(ZoomIn);
}


uint KStdAccel::zoomOut()
{
  return key(ZoomOut);
}


uint KStdAccel::insert()
{
  return key(Insert);
}

uint KStdAccel::home()
{
  return key(Home);
}

uint KStdAccel::end()
{
  return key(End);
}

uint KStdAccel::prior()
{
  return key(Prior);
}

uint KStdAccel::next()
{
  return key(Next);
}

uint KStdAccel::gotoLine()
{
  return key(GotoLine);
}

uint KStdAccel::addBookmark()
{
  return key(AddBookmark);
}

uint KStdAccel::help()
{
  return key(Help);
}

uint KStdAccel::completion()
{
  return key(TextCompletion);
}

uint KStdAccel::prevCompletion()
{
  return key(PrevCompletion);
}

uint KStdAccel::nextCompletion()
{
  return key(NextCompletion);
}

uint KStdAccel::rotateUp()
{
  return key(RotateUp);
}

uint KStdAccel::rotateDown()
{
  return key(RotateDown);
}

uint KStdAccel::popupMenuContext()
{
  return key(PopupMenuContext);
}

uint KStdAccel::whatsThis()
{
  return key(WhatThis);
}

uint KStdAccel::reload()
{
  return key(Reload);
}

uint KStdAccel::selectAll()
{
  return key(SelectAll);
}

uint KStdAccel::up()
{
  return key(Up);
}

uint KStdAccel::back()
{
  return key(Back);
}

uint KStdAccel::forward()
{
  return key(Forward);
}

uint KStdAccel::showMenubar()
{
  return key(ShowMenubar);
}
