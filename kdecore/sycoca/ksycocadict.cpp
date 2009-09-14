/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "ksycocadict.h"
#include <kservice.h>
#include "ksycocaentry.h"
#include "ksycoca.h"
#include "kdebug.h"

#include <QtCore/QBitArray>

namespace
{
struct string_entry {
    string_entry(const QString& _key, const KSycocaEntry::Ptr& _payload)
      : hash(0), length(_key.length()), keyStr(_key), key(keyStr.unicode()), payload(_payload)
    {}
    uint hash;
    const int length;
    const QString keyStr;
    const QChar * const key; // always points to keyStr.unicode(); just an optimization
    const KSycocaEntry::Ptr payload;
};
}

class KSycocaDictStringList : public QList<string_entry*>
{
public:
   ~KSycocaDictStringList() {
       qDeleteAll(*this);
   }
};

class KSycocaDict::Private
{
public:
    Private()
        : stringlist( 0 ),
          stream( 0 ),
          offset( 0 )
    {
    }

    ~Private()
    {
        delete stringlist;
    }

    // Helper for find_string and findMultiString
    qint32 offsetForKey(const QString& key) const;

    // Calculate hash - can be used during loading and during saving.
    quint32 hashKey(const QString & key) const;

    KSycocaDictStringList *stringlist;
    QDataStream *stream;
    qint64 offset;
    quint32 hashTableSize;
    QList<qint32> hashList;
};

KSycocaDict::KSycocaDict()
  : d( new Private )
{
}

KSycocaDict::KSycocaDict(QDataStream *str, int offset)
  : d( new Private )
{
   d->stream = str;
   d->offset = offset;

   quint32 test1, test2;
   str->device()->seek(offset);
   (*str) >> test1 >> test2;
   if ((test1 > 0x000fffff) || (test2 > 1024))
   {
       KSycoca::flagError();
       d->hashTableSize = 0;
       d->offset = 0;
       return;
   }

   str->device()->seek(offset);
   (*str) >> d->hashTableSize;
   (*str) >> d->hashList;
   d->offset = str->device()->pos(); // Start of hashtable
}

KSycocaDict::~KSycocaDict()
{
   delete d;
}

void
KSycocaDict::add(const QString &key, const KSycocaEntry::Ptr& payload)
{
   if (key.isEmpty()) return; // Not allowed (should never happen)
   if (!payload) return; // Not allowed!
   if (!d->stringlist)
   {
       d->stringlist = new KSycocaDictStringList;
   }

   string_entry *entry = new string_entry(key, payload);
   d->stringlist->append(entry);
}

void
KSycocaDict::remove(const QString &key)
{
    if (!d || !d->stringlist) {
        return;
    }

   bool found = false;
   for(KSycocaDictStringList::Iterator it = d->stringlist->begin(); it != d->stringlist->end(); ++it) {
      string_entry* entry = *it;
      if (entry->keyStr == key) {
         d->stringlist->erase(it);
         delete entry;
         found = true;
         break;
      }
   }
   if (!found) {
       kWarning(7011) << "key not found:" << key;
   }
}

int KSycocaDict::find_string(const QString &key ) const
{
    Q_ASSERT(d);

    //kDebug(7011) << QString("KSycocaDict::find_string(%1)").arg(key);
    qint32 offset = d->offsetForKey(key);

    //kDebug(7011) << QString("offset is %1").arg(offset,8,16);
    if (offset == 0)
        return 0;

    if (offset > 0)
        return offset; // Positive ID

    // Lookup duplicate list.
    offset = -offset;

    d->stream->device()->seek(offset);
    //kDebug(7011) << QString("Looking up duplicate list at %1").arg(offset,8,16);

   while(true)
   {
       (*d->stream) >> offset;
       if (offset == 0) break;
       QString dupkey;
       (*d->stream) >> dupkey;
       //kDebug(7011) << QString(">> %1 %2").arg(offset,8,16).arg(dupkey);
       if (dupkey == key) return offset;
   }
   //kWarning(7011) << "Not found!";

   return 0;
}


QList<int> KSycocaDict::findMultiString(const QString &key ) const
{
    qint32 offset = d->offsetForKey(key);
    QList<int> offsetList;
    if (offset == 0)
        return offsetList;

    if (offset > 0) { // Positive ID: one entry found
        offsetList.append(offset);
        return offsetList;
    }

    // Lookup duplicate list.
    offset = -offset;

    d->stream->device()->seek(offset);
    //kDebug(7011) << QString("Looking up duplicate list at %1").arg(offset,8,16);

    while(true)
    {
        (*d->stream) >> offset;
        if (offset == 0) break;
        QString dupkey;
        (*d->stream) >> dupkey;
        //kDebug(7011) << QString(">> %1 %2").arg(offset,8,16).arg(dupkey);
        if (dupkey == key)
            offsetList.append(offset);
    }
    return offsetList;
}

uint KSycocaDict::count() const
{
   if ( !d || !d->stringlist ) return 0;

   return d->stringlist->count();
}

void
KSycocaDict::clear()
{
   delete d;
   d = 0;
}

uint KSycocaDict::Private::hashKey( const QString &key) const
{
   int l = key.length();
   register uint h = 0;

   for(int i = 0; i < hashList.count(); i++)
   {
      int pos = hashList[i];
      if (pos < 0)
      {
         pos = -pos-1;
         if (pos < l)
            h = ((h * 13) + (key[l-pos].cell() % 29)) & 0x3ffffff;
      }
      else
      {
         pos = pos-1;
         if (pos < l)
            h = ((h * 13) + (key[pos].cell() % 29)) & 0x3ffffff;
      }
   }
   return h;
}

//
// Calculate the diversity of the strings at position 'pos'
static int
calcDiversity(KSycocaDictStringList *stringlist, int pos, int sz)
{
   if (pos == 0) return 0;
   QBitArray matrix(sz);
   uint usz = sz;

   if (pos < 0)
   {
      pos = -pos-1;
      for(KSycocaDictStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
      {
        string_entry* entry = *it;
	register int l = entry->length;
         if (pos < l && pos != 0)
         {
           register uint hash = ((entry->hash * 13) + (entry->key[l-pos].cell() % 29)) & 0x3ffffff;
	   matrix.setBit( hash % usz, true );
         }
      }
   }
   else
   {
      pos = pos-1;
      for(KSycocaDictStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
      {
         string_entry* entry = *it;
         if (pos < entry->length)
         {
            register uint hash = ((entry->hash * 13) + (entry->key[pos].cell() % 29)) & 0x3ffffff;
	    matrix.setBit( hash % usz, true );
         }
      }
   }
   int diversity = 0;
   for(int i=0;i< sz;i++)
      if (matrix.testBit(i)) diversity++;

   return diversity;
}

//
// Add the diversity of the strings at position 'pos'
static void
addDiversity(KSycocaDictStringList *stringlist, int pos)
{
   if (pos == 0) return;
   if (pos < 0)
   {
      pos = -pos-1;
      for(KSycocaDictStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
      {
         string_entry* entry = *it;
         register int l = entry->length;
         if (pos < l)
            entry->hash = ((entry->hash * 13) + (entry->key[l-pos].cell() % 29)) & 0x3fffffff;
      }
   }
   else
   {
      pos = pos - 1;
      for(KSycocaDictStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
      {
         string_entry* entry = *it;
         if (pos < entry->length)
            entry->hash = ((entry->hash * 13) + (entry->key[pos].cell() % 29)) & 0x3fffffff;
      }
   }
}


void
KSycocaDict::save(QDataStream &str)
{
   if (count() == 0)
   {
      d->hashTableSize = 0;
      d->hashList.clear();
      str << d->hashTableSize;
      str << d->hashList;
      return;
   }

   d->offset = str.device()->pos();

   //kDebug(7011) << QString("KSycocaDict: %1 entries.").arg(count());

   //kDebug(7011) << "Calculating hash keys..";

   int maxLength = 0;
   //kDebug(7011) << "Finding maximum string length";
   for(KSycocaDictStringList::Iterator it = d->stringlist->begin(); it != d->stringlist->end(); ++it)
   {
      string_entry* entry = *it;
      entry->hash = 0;
      if (entry->length > maxLength)
         maxLength = entry->length;
   }

   //kDebug(7011) << QString("Max string length = %1").arg(maxLength);

   // use "almost prime" number for sz (to calculate diversity) and later
   // for the table size of big tables
   // int sz = d->stringlist->count()*5-1;
   register unsigned int sz = count()*4 + 1;
   while(!(((sz % 3) && (sz % 5) && (sz % 7) && (sz % 11) && (sz % 13))))
      sz+=2;

   int maxDiv = 0;
   int maxPos = 0;
   int lastDiv = 0;

   d->hashList.clear();

   // try to limit diversity scan by "predicting" positions
   // with high diversity
   int *oldvec=new int[maxLength*2+1];
   for (int i=0; i<(maxLength*2+1); i++) oldvec[i]=0;
   int mindiv=0;

   while(true)
   {
      int divsum=0,divnum=0;

      maxDiv = 0;
      maxPos = 0;
      for(int pos=-maxLength; pos <= maxLength; pos++)
      {
         // cut off
         if (oldvec[pos+maxLength]<mindiv)
         { oldvec[pos+maxLength]=0; continue; }

         int diversity = calcDiversity(d->stringlist, pos, sz);
         if (diversity > maxDiv)
         {
            maxDiv = diversity;
            maxPos = pos;
         }
         oldvec[pos+maxLength]=diversity;
         divsum+=diversity; divnum++;
      }
      // arbitrary cut-off value 3/4 of average seems to work
      if (divnum)
         mindiv=(3*divsum)/(4*divnum);

      if (maxDiv <= lastDiv)
         break;
      // qWarning("Max Div = %d at pos %d", maxDiv, maxPos);
      lastDiv = maxDiv;
      addDiversity(d->stringlist, maxPos);
      d->hashList.append(maxPos);
   }

   delete [] oldvec;

   for(KSycocaDictStringList::Iterator it = d->stringlist->begin(); it != d->stringlist->end(); ++it)
      (*it)->hash = d->hashKey((*it)->keyStr);
// fprintf(stderr, "Calculating minimum table size..\n");

   d->hashTableSize = sz;

   struct hashtable_entry {
      string_entry *entry;
      QList<string_entry*>* duplicates;
      qint64 duplicate_offset;
   };

   hashtable_entry *hashTable = new hashtable_entry[ sz ];

   //kDebug(7011) << "Clearing hashtable...";
   for (unsigned int i=0; i < sz; i++)
   {
      hashTable[i].entry = 0;
      hashTable[i].duplicates = 0;
   }

   //kDebug(7011) << "Filling hashtable...";
   for(KSycocaDictStringList::Iterator it = d->stringlist->begin(); it != d->stringlist->end(); ++it)
   {
      string_entry* entry = *it;
      //kDebug(7011) << "entry keyStr=" << entry->keyStr << entry->payload.data() << entry->payload->entryPath();
      int hash = entry->hash % sz;
      if (!hashTable[hash].entry)
      { // First entry
         hashTable[hash].entry = entry;
      }
      else
      {
         if (!hashTable[hash].duplicates)
         { // Second entry, build duplicate list.
            hashTable[hash].duplicates = new QList<string_entry*>;
            hashTable[hash].duplicates->append(hashTable[hash].entry);
            hashTable[hash].duplicate_offset = 0;
         }
         hashTable[hash].duplicates->append(entry);
      }
   }

   str << d->hashTableSize;
   str << d->hashList;

   d->offset = str.device()->pos(); // d->offset points to start of hashTable
   //kDebug(7011) << QString("Start of Hash Table, offset = %1").arg(d->offset,8,16);

   // Write the hashtable + the duplicates twice.
   // The duplicates are after the normal hashtable, but the offset of each
   // duplicate entry is written into the normal hashtable.
   for(int pass = 1; pass <= 2; pass++)
   {
      str.device()->seek(d->offset);
      //kDebug(7011) << QString("Writing hash table (pass #%1)").arg(pass);
      for(uint i=0; i < d->hashTableSize; i++)
      {
         qint32 tmpid;
         if (!hashTable[i].entry)
            tmpid = (qint32) 0;
         else if (!hashTable[i].duplicates)
            tmpid = (qint32) hashTable[i].entry->payload->offset(); // Positive ID
         else
            tmpid = (qint32) -hashTable[i].duplicate_offset; // Negative ID
         str << tmpid;
         //kDebug(7011) << QString("Hash table : %1").arg(tmpid,8,16);
      }
      //kDebug(7011) << QString("End of Hash Table, offset = %1").arg(str.device()->at(),8,16);

      //kDebug(7011) << QString("Writing duplicate lists (pass #%1)").arg(pass);
      for(uint i=0; i < d->hashTableSize; i++)
      {
         const QList<string_entry*> *dups = hashTable[i].duplicates;
         if (dups)
         {
            hashTable[i].duplicate_offset = str.device()->pos();

            /*kDebug(7011) << QString("Duplicate lists: Offset = %1 list_size = %2")                           .arg(hashTable[i].duplicate_offset,8,16).arg(dups->count());
*/
	    for(QList<string_entry*>::ConstIterator dup = dups->begin(); dup != dups->end(); ++dup)
            {
               const qint32 offset = (*dup)->payload->offset();
               if (!offset) {
                   const QString storageId = (*dup)->payload->storageId();
                   kDebug() << "about to assert! dict=" << this << "storageId=" << storageId << (*dup)->payload.data();
                   if ((*dup)->payload->isType(KST_KService)) {
                       KService::Ptr service = KService::Ptr::staticCast((*dup)->payload);
                       kDebug() << service->storageId() << service->entryPath();
                   }
                   // save() must have been called on the entry
                   Q_ASSERT_X( offset, "KSycocaDict::save",
                               QByteArray("entry offset is 0, save() was not called on ")
                               + (*dup)->payload->storageId().toLatin1()
                               + " entryPath="
                               + (*dup)->payload->entryPath().toLatin1()
                       );
               }
               str << offset ;                       // Positive ID
               str << (*dup)->keyStr;                // Key (QString)
            }
            str << (qint32) 0;               // End of list marker (0)
         }
      }
      //kDebug(7011) << QString("End of Dict, offset = %1").arg(str.device()->at(),8,16);
   }

   //kDebug(7011) << "Cleaning up hash table.";
   for(uint i=0; i < d->hashTableSize; i++)
   {
      delete hashTable[i].duplicates;
   }
   delete [] hashTable;
}

qint32 KSycocaDict::Private::offsetForKey(const QString& key) const
{
   if ( !stream || !offset )
   {
      kError() << "No ksycoca4 database available!" << endl;
      return 0;
   }

   if (hashTableSize == 0)
      return 0; // Unlikely to find anything :-]

   // Read hash-table data
   const uint hash = hashKey(key) % hashTableSize;
   //kDebug(7011) << "hash is" << hash;

   const qint32 off = offset+sizeof(qint32)*hash;
   //kDebug(7011) << QString("off is %1").arg(off,8,16);
   stream->device()->seek( off );

   qint32 retOffset;
   (*stream) >> retOffset;
   return retOffset;
}
