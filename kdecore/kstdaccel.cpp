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

KStdAccel::KStdAccel(KConfig* /* cfg */) //: KKeyConfig(cfg)
{
  config = new KConfig; // we want the global stuff, and only the global stuff
  config->setGroup("Standard Keys");
}

KStdAccel::~KStdAccel()
{
    delete config;
}

uint KStdAccel::readKey(KAccel::StdAccel accel, uint defaultKey) const
{
  QString s = config->readEntry(KAccel::stdAction(accel));
  int v = KAccel::stringToKey(s);
  return v?v:defaultKey;

}

uint KStdAccel::open() const
{
  return readKey(KAccel::Open, Qt::CTRL + Qt::Key_O);
}

uint KStdAccel::openNew() const
{
  return readKey(KAccel::New, Qt::CTRL+Qt::Key_N);
}

uint KStdAccel::close() const
{
  return readKey(KAccel::Close, Qt::CTRL+Qt::Key_W);
}

uint KStdAccel::save() const
{
  return readKey(KAccel::Save, Qt::CTRL+Qt::Key_S);
}

uint KStdAccel::print() const
{
  return readKey(KAccel::Print, Qt::CTRL+Qt::Key_P);
}

uint KStdAccel::quit() const
{
  return readKey(KAccel::Quit, Qt::CTRL+Qt::Key_Q);
}

uint KStdAccel::cut() const
{
  return readKey(KAccel::Cut, Qt::CTRL+Qt::Key_X);
}

uint KStdAccel::copy() const
{
  return readKey(KAccel::Copy, Qt::CTRL+Qt::Key_C);
}

uint KStdAccel::paste() const
{
  return readKey(KAccel::Paste, Qt::CTRL+Qt::Key_V);
}

uint KStdAccel::undo() const
{
  return readKey(KAccel::Undo, Qt::CTRL+Qt::Key_Z);
}

uint KStdAccel::redo() const
{
  return readKey(KAccel::Redo, Qt::CTRL+Qt::Key_Y);
}

uint KStdAccel::find() const
{
  return readKey(KAccel::Find, Qt::CTRL+Qt::Key_F);
}

uint KStdAccel::replace() const
{
  return readKey(KAccel::Replace, Qt::CTRL+Qt::Key_R);
}

uint KStdAccel::insert() const
{
  return readKey(KAccel::Insert, Qt::CTRL+Qt::Key_Insert);
}

uint KStdAccel::home() const
{
  return readKey(KAccel::Home, Qt::CTRL+Qt::Key_Home);
}

uint KStdAccel::end() const
{
  return readKey(KAccel::End, Qt::CTRL+Qt::Key_End);
}

uint KStdAccel::prior() const
{
  return readKey(KAccel::Prior, Qt::Key_Prior);
}

uint KStdAccel::next() const
{
  return readKey(KAccel::Next, Qt::Key_Next);
}

uint KStdAccel::help() const
{
  return readKey(KAccel::Help, Qt::Key_F1);
}

