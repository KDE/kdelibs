
#include <kstaticdeleter.h>

// this helps gcc to emit the vtbl for KStaticDeleterBase
// only once, here in this file, not every time it's
// used, says Seli.
void KStaticDeleterBase::destructObject()
{
}

