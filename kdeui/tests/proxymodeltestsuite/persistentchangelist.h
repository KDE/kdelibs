
#ifndef PERSISTENTCHANGELIST_H
#define PERSISTENTCHANGELIST_H

#include "indexfinder.h"

struct PersistentIndexChange
{
  IndexFinder parentFinder;
  int startRow;
  int endRow;
  int difference;
  bool toInvalid;
  QModelIndexList indexes;
  QList<QPersistentModelIndex> persistentIndexes;

  QModelIndexList descendantIndexes;
  QList<QPersistentModelIndex> persistentDescendantIndexes;
};

typedef QList<PersistentIndexChange> PersistentChangeList;

Q_DECLARE_METATYPE( PersistentChangeList )

#endif