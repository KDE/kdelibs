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
  int v = stringToKey(s.data());
  return v?v:defaultKey;

}

uint KStdAccel::open(void) const
{
  return readKey(KAccel::Open, Qt::CTRL + Qt::Key_O);
}

uint KStdAccel::openNew(void) const
{
  return readKey(KAccel::New, Qt::CTRL+Qt::Key_N);
}

uint KStdAccel::close(void) const
{
  return readKey(KAccel::Close, Qt::CTRL+Qt::Key_W);
}

uint KStdAccel::save(void) const
{
  return readKey(KAccel::Save, Qt::CTRL+Qt::Key_S);
}

uint KStdAccel::print(void) const
{
  return readKey(KAccel::Print, Qt::CTRL+Qt::Key_P);
}

uint KStdAccel::quit(void) const
{
  return readKey(KAccel::Quit, Qt::CTRL+Qt::Key_Q);
}

uint KStdAccel::cut(void) const
{
  return readKey(KAccel::Cut, Qt::CTRL+Qt::Key_X);
}

uint KStdAccel::copy(void) const
{
  return readKey(KAccel::Copy, Qt::CTRL+Qt::Key_C);
}

uint KStdAccel::paste(void) const
{
  return readKey(KAccel::Paste, Qt::CTRL+Qt::Key_V);
}

uint KStdAccel::undo(void) const
{
  return readKey(KAccel::Undo, Qt::CTRL+Qt::Key_Z);
}

uint KStdAccel::redo(void) const
{
  return readKey(KAccel::Redo, Qt::CTRL+Qt::Key_Y);
}

uint KStdAccel::find(void) const
{
  return readKey(KAccel::Find, Qt::CTRL+Qt::Key_F);
}

uint KStdAccel::replace(void) const
{
  return readKey(KAccel::Replace, Qt::CTRL+Qt::Key_R);
}

uint KStdAccel::insert(void) const
{
  return readKey(KAccel::Insert, Qt::CTRL+Qt::Key_Insert);
}

uint KStdAccel::home(void) const
{
  return readKey(KAccel::Home, Qt::CTRL+Qt::Key_Home);
}

uint KStdAccel::end(void) const
{
  return readKey(KAccel::End, Qt::CTRL+Qt::Key_End);
}

uint KStdAccel::prior(void) const
{
  return readKey(KAccel::Prior, Qt::Key_Prior);
}

uint KStdAccel::next(void) const
{
  return readKey(KAccel::Next, Qt::Key_Next);
}

uint KStdAccel::help(void) const
{
  return readKey(KAccel::Help, Qt::Key_F1);
}

