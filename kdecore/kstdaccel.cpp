/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@alpin.or.at)

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

#include "kstdaccel.h"
#include "kapp.h"
#include "kconfig.h"
#include "kglobal.h"

KStdAccel::KStdAccel()
{
}

KStdAccel::~KStdAccel()
{
}

uint KStdAccel::readKey(KAccel::StdAccel accel, uint defaultKey)
{
    KConfigGroupSaver saver ( KGlobal::config(), "Keys" ) ;
    QString s = saver.config()->readEntry(KAccel::stdAction(accel));
    int v = KAccel::stringToKey(s);
    return v?v:defaultKey;
}

uint KStdAccel::open()
{
  return readKey(KAccel::Open, Qt::CTRL + Qt::Key_O);
}

uint KStdAccel::openNew()
{
  return readKey(KAccel::New, Qt::CTRL+Qt::Key_N);
}

uint KStdAccel::close()
{
  return readKey(KAccel::Close, Qt::CTRL+Qt::Key_W);
}

uint KStdAccel::save()
{
  return readKey(KAccel::Save, Qt::CTRL+Qt::Key_S);
}

uint KStdAccel::print()
{
  return readKey(KAccel::Print, Qt::CTRL+Qt::Key_P);
}

uint KStdAccel::quit()
{
  return readKey(KAccel::Quit, Qt::CTRL+Qt::Key_Q);
}

uint KStdAccel::cut()
{
  return readKey(KAccel::Cut, Qt::CTRL+Qt::Key_X);
}

uint KStdAccel::copy()
{
  return readKey(KAccel::Copy, Qt::CTRL+Qt::Key_C);
}

uint KStdAccel::paste()
{
  return readKey(KAccel::Paste, Qt::CTRL+Qt::Key_V);
}

uint KStdAccel::undo()
{
  return readKey(KAccel::Undo, Qt::CTRL+Qt::Key_Z);
}

uint KStdAccel::redo()
{
  return readKey(KAccel::Redo, Qt::SHIFT+Qt::CTRL+Qt::Key_Z);
}

uint KStdAccel::find()
{
  return readKey(KAccel::Find, Qt::CTRL+Qt::Key_F);
}

uint KStdAccel::findNext()
{
  return readKey(KAccel::FindNext, Qt::Key_F3);
}

uint KStdAccel::findPrev()
{
  return readKey(KAccel::FindPrev, Qt::SHIFT+Qt::Key_F3);
}

uint KStdAccel::replace()
{
  return readKey(KAccel::Replace, Qt::CTRL+Qt::Key_R);
}


uint KStdAccel::zoomIn()
{
  return readKey(KAccel::ZoomIn, Qt::CTRL+Qt::Key_Plus);
}


uint KStdAccel::zoomOut()
{
  return readKey(KAccel::ZoomOut, Qt::CTRL+Qt::Key_Minus);
}


uint KStdAccel::insert()
{
  return readKey(KAccel::Insert, Qt::CTRL+Qt::Key_Insert);
}

uint KStdAccel::home()
{
  return readKey(KAccel::Home, Qt::CTRL+Qt::Key_Home);
}

uint KStdAccel::end()
{
  return readKey(KAccel::End, Qt::CTRL+Qt::Key_End);
}

uint KStdAccel::prior()
{
  return readKey(KAccel::Prior, Qt::Key_Prior);
}

uint KStdAccel::next()
{
  return readKey(KAccel::Next, Qt::Key_Next);
}

uint KStdAccel::addBookmark()
{
  return readKey(KAccel::AddBookmark, Qt::CTRL+Qt::Key_B);
}

uint KStdAccel::help()
{
  return readKey(KAccel::Help, Qt::Key_F1);
}

