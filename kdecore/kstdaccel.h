/* Convenient methods for access of the common shortcut keys in
 * the key configuration.
 *
 * Author: Stefan Taferner <taferner@salzburg.co.at>
 */
#ifndef KSTDACCEL_H
#define KSTDACCEL_H

#include <kkeyconf.h>

class KStdAccel: public KKeyConfig
{
public:
  KStdAccel(KConfig* pConfig);

  uint open(void) const;
  uint openNew(void) const;
  uint close(void) const;
  uint save(void) const;
  uint print(void) const;
  uint quit(void) const;
  uint cut(void) const;
  uint copy(void) const;
  uint paste(void) const;
  uint undo(void) const;
  uint find(void) const;
  uint replace(void) const;
  uint insert(void) const;
  uint home(void) const;
  uint end(void) const;
  uint prior(void) const;
  uint next(void) const;
  uint help(void) const;

protected:
  uint readKey(const char* keyName, uint defaultKey=0) const;
};

#endif /*kshortcut_h*/
