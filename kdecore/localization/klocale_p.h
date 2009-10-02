#ifndef KLOCALE_P_H
#define KLOCALE_P_H

class QMutex;

// Used by both KLocale and KLocalizedString, since they call each other.
QMutex* kLocaleMutex();

#endif /* KLOCALE_P_H */

