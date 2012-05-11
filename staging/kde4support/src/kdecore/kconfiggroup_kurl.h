#ifndef KCONFIGGROUP_KURL_H
#define KCONFIGGROUP_KURL_H

#include <kurl.h>
#include <kconfiggroup.h>

/// This header defines support for KUrl in KConfigGroup (readEntry/writeEntry)

template <>
inline void KConfigGroup::writeCheck(const char* key, const KUrl& value, KConfigBase::WriteConfigFlags flags) {
    writeEntry(key, value.url(), flags);
}

template <>
inline KUrl KConfigGroup::readCheck(const char* key, const KUrl& defaultValue) const {
    return KUrl(readEntry(key, defaultValue.url()));
}

#endif /* KCONFIGGROUP_KURL_H */

