// kstdaccel.cpp

#include "kstdaccel.h"

KStdAccel::KStdAccel(KConfig* cfg): KKeyConfig(cfg)
{
}

uint KStdAccel::readKey(const char* keyName, uint defaultKey) const
{
  uint key;
  key = ((KStdAccel*)this)->readCurrentKey(keyName);
  return (key ? key : defaultKey);
}

uint KStdAccel::open(void) const
{
  return readKey("Open", CTRL+Key_O);
}

uint KStdAccel::openNew(void) const
{
  return readKey("OpenNew", CTRL+Key_N);
}

uint KStdAccel::close(void) const
{
  return readKey("Close", CTRL+Key_W);
}

uint KStdAccel::save(void) const
{
  return readKey("Save", CTRL+Key_S);
}

uint KStdAccel::print(void) const
{
  return readKey("Print", CTRL+Key_P);
}

uint KStdAccel::quit(void) const
{
  return readKey("Quit", CTRL+Key_Q);
}

uint KStdAccel::cut(void) const
{
  return readKey("Cut", CTRL+Key_X);
}

uint KStdAccel::copy(void) const
{
  return readKey("Copy", CTRL+Key_C);
}

uint KStdAccel::paste(void) const
{
  return readKey("Paste", CTRL+Key_V);
}

uint KStdAccel::undo(void) const
{
  return readKey("Undo", CTRL+Key_Z);
}

uint KStdAccel::find(void) const
{
  return readKey("Find", CTRL+Key_F);
}

uint KStdAccel::replace(void) const
{
  return readKey("Replace", CTRL+Key_R);
}

uint KStdAccel::insert(void) const
{
  return readKey("Insert", CTRL+Key_Insert);
}

uint KStdAccel::home(void) const
{
  return readKey("Home", CTRL+Key_Home);
}

uint KStdAccel::end(void) const
{
  return readKey("End", CTRL+Key_End);
}

uint KStdAccel::prior(void) const
{
  return readKey("Prior", Key_Prior);
}

uint KStdAccel::next(void) const
{
  return readKey("Next", Key_Next);
}

uint KStdAccel::help(void) const
{
  return readKey("Help", Key_F1);
}

