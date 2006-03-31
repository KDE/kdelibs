#ifndef _KGLOBALACCEL_MAC_H
#define _KGLOBALACCEL_MAC_H

#warning KGlobalAccel is a no-op on OSX right now, FIX IT!

#include <qwidget.h>

#include "kshortcut.h"

class KGlobalAccelImpl: public QWidget
{
	Q_OBJECT
public:
	KGlobalAccelImpl(class KGlobalAccel* owner);

public:
	/**
	 * This function registers or unregisters a certain key for global capture,
	 * depending on \b grab.
	 *
	 * Before destruction, every grabbed key will be released, so this
	 * object does not need to do any tracking.
	 *
	 * \param key the Qt keycode to grab or release.
	 * \param grab true to grab they key, false to release the key.
	 *
	 * \return true if successful, otherwise false.
	 */
	bool grabKey(int key, bool grab);
	
	/// Enable all shortcuts.  There will not be any grabbed shortcuts at this point.
	void enable();

	/// Disable all shortcuts.  There will not be any grabbed shortcuts at this point.
	void disable();

private:
	KGlobalAccel* m_owner;

};

#endif // _KGLOBALACCEL_MAC_H
