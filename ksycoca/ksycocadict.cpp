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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <ksycocadict.h>
#include <ksycocaentry.h>

#include <qlist.h>
#include <qvaluelist.h>
#include <kdebug.h>

// DEBUG only
#include <stdio.h>

struct string_entry {
  string_entry(QString _key, KSycocaEntry *_payload) 
     { key = _key; payload = _payload; }
  QString key;
  KSycocaEntry *payload;
  uint hash;
};

class KSycocaDictStringList : public QList<string_entry>
{
public:
   KSycocaDictStringList();
};

KSycocaDictStringList::KSycocaDictStringList()
{
   setAutoDelete(true);
}

KSycocaDict::KSycocaDict()
  : d(0), mStr(0), mOffset(0)
{
}

KSycocaDict::KSycocaDict(QDataStream *str, int offset)
  : d(0), mStr(str), mOffset(offset)
{
   str->device()->at(offset);
   (*str) >> mHashTableSize;
   (*str) >> mHashList;
   mOffset = str->device()->at(); // Start of hashtable
}

KSycocaDict::~KSycocaDict()
{
   delete d;
}

void 
KSycocaDict::add(const QString &key, KSycocaEntry *payload)
{
   if (key.isEmpty()) return; // Not allowed (should never happen)
   if (!payload) return; // Not allowed!
   if (!d)
   {
       d = new KSycocaDictStringList();
   }

   string_entry *entry= new string_entry(key, payload);
   d->append(entry);
}
   
KSycocaEntry * KSycocaDict::findEntryInMemory( const QString & key )
{
  if ( !d )
    return 0L;

  // This is a building database. Use the in-memory list.
  // VERY SLOW. But who cares. :)
  // Used for looking up the mimetype(s) when reading services...
  // Well we might want to replace the list with a dict...
  QListIterator<string_entry> it ( *d );
  for ( ; it.current(); ++it )
  {
    if ( it.current()->key == key )
      return it.current()->payload;
  }
  return 0L;
}

int 
KSycocaDict::find_string(const QString &key )
{
   kdebug(KDEBUG_INFO, 7011, QString("KSycocaDict::find_string(%1)").arg(key));

   if (!mStr || !mOffset)
   {
      fprintf(stderr, "No database available!\n");
      return 0;
   }

   // Read hash-table data 
   uint hash = hashKey(key) % mHashTableSize;
   kdebug(KDEBUG_INFO, 7011, QString("hash is %1").arg(hash));

   uint off = mOffset+sizeof(Q_INT32)*hash;
   kdebug(KDEBUG_INFO, 7011, QString("off is %1").arg(hash));
   mStr->device()->at( off );

   Q_INT32 offset;
   (*mStr) >> offset;

   kdebug(KDEBUG_INFO, 7011, QString("offset is %1").arg(offset));
   if (offset == 0)
      return 0;

   if (offset > 0)
      return offset; // Positive ID

   // Lookup duplicate list.
   offset = -offset;

   mStr->device()->at(offset);
fprintf(stderr, "Looking up duplicate list at %08x\n", offset);
   
   while(true)
   {
       (*mStr) >> offset;
       if (offset == 0) break;
       QString dupkey;
       (*mStr) >> dupkey;
fprintf(stderr, ">> %08x %s\n", offset, dupkey.ascii());
       if (dupkey == key) return offset;
   }
fprintf(stderr, "Not found!\n");

   return 0;
}
   
uint 
KSycocaDict::count()
{
   if (!d) return 0;

   return d->count();
}

void 
KSycocaDict::clear()
{
   delete d;
   d = 0;
}

uint
KSycocaDict::hashKey( const QString &key)
{
   int l = key.length();
   uint h = 0;
  
   for(uint i = 0; i < mHashList.count(); i++)
   {
      int pos = mHashList[i];
      if (pos < 0)
      {
         pos = -pos-1;
         if (pos < l)
            h = (h * 13) + (key[l-pos].cell() % 29);
         h = h & 0x3ffffff;
      } 
      else
      {
         pos = pos-1;
         if (pos < l)
            h = (h * 13) + (key[pos].cell() % 29);
         h = h & 0x3ffffff;
      }
   }
   return h;
}

//
// Calculate the diversity of the strings at position 'pos'
int 
calcDiversity(KSycocaDictStringList *d, int pos, int sz)
{
   if (pos == 0) return 0;
   bool *matrix = new bool[sz];
 
   for(int i=sz;i--;)
      matrix[i] = false;

   if (pos < 0)
   {
      pos = -pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         register int l = entry->key.length();
         if (pos < l)
         {
            uint hash = (entry->hash * 13) + (entry->key[l-pos].cell() % 29);
            hash = hash & 0x3ffffff;
            matrix[ hash % sz ] = true;
         }
      }
   }
   else
   {
      pos = pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         register int l = entry->key.length();
         if (pos < l)
         {
            uint hash = (entry->hash * 13) + (entry->key[pos].cell() % 29);
            hash = hash & 0x3ffffff;
            matrix[ hash % sz ] = true;
         }
      }
   }
   int diversity = 0;
   for(int i=0;i< sz;i++)
      if (matrix[i]) diversity++;
   
   delete [] matrix;

   return diversity;
}

//
// Add the diversity of the strings at position 'pos'
void 
addDiversity(KSycocaDictStringList *d, int pos)
{
   if (pos == 0) return;
   if (pos < 0)
   {
      pos = -pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         register int l = entry->key.length();
         if (pos < l)
            entry->hash = (entry->hash * 13) + (entry->key[l-pos].cell() % 29);
         entry->hash = entry->hash & 0x3fffffff;
      }
   }
   else
   {
      pos = pos - 1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         register int l = entry->key.length();
         if (pos < l)
            entry->hash = (entry->hash * 13) + (entry->key[pos].cell() % 29);
         entry->hash = entry->hash & 0x3fffffff;
      }
   }
}


void 
KSycocaDict::save(QDataStream &str)
{
   if (!d)
   {
       d = new KSycocaDictStringList();
   }

   mOffset = str.device()->at();

fprintf(stderr, "KSycocaDict: %d entries.\n", count());

fprintf(stderr, "Calculating hash keys..\n");

   int maxLength = 0;
   fprintf(stderr, "Finding maximum string length\n");
   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
      entry->hash = 0;
//fprintf( stderr, "%s\n", entry->key.ascii() );
      if ((int) entry->key.length() > maxLength)
         maxLength = entry->key.length();
   }
   
   fprintf(stderr, "Max string length = %d\n", maxLength);

   int sz = d->count()*5-1;
   int maxDiv = 0;
   int maxPos = 0;
   int lastDiv = 0;

   mHashList.clear();

   while(true)
   {
      maxDiv = 0;
      maxPos = 0;
      for(int pos=-maxLength; pos <= maxLength; pos++)
      {
         int diversity = calcDiversity(d, pos, sz);
         if (diversity > maxDiv)
         {
            maxDiv = diversity;
            maxPos = pos;
         }
      }
      if (maxDiv <= lastDiv)
         break;
//    fprintf(stderr, "Max Div = %d at pos %d\n", maxDiv, maxPos);
      lastDiv = maxDiv;
      addDiversity(d, maxPos); 
      mHashList.append(maxPos);
   }

   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
      entry->hash = hashKey(entry->key);
   }

// fprintf(stderr, "Calculating minimum table size..\n");
   mHashTableSize = 0;
   int minDups = count();
   uint maxHashTableSize = count()*5;
   for(uint hashTableSize = count(); hashTableSize < maxHashTableSize; hashTableSize++)
   {
      int *checkList = new int[hashTableSize];
      for(uint i = 0; i < hashTableSize; i++)
         checkList[i] = 0;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         checkList[entry->hash % hashTableSize]++;
      }
       
      int dups = 0;
      for(uint i = 0; i < hashTableSize; i++)
      {
         if (checkList[i] > 1)
            dups += (checkList[i]-1)*(checkList[i]-1);
      }
      if (dups < minDups)
      {
          mHashTableSize = hashTableSize;
          minDups = dups;
      }
      delete [] checkList;
   }
   fprintf(stderr, "item count = %d min. dups = %d, hashtable size = %d\n",
		d->count(), minDups, mHashTableSize);

   struct hashtable_entry {
      string_entry *entry;
      QList<string_entry> *duplicates;
      int duplicate_offset;
   };

   hashtable_entry *hashTable = new hashtable_entry[ mHashTableSize ];

fprintf(stderr, "Clearing hashtable...\n");
   for(uint i=0; i < mHashTableSize; i++)
   {
      hashTable[i].entry = 0;
      hashTable[i].duplicates = 0;
   }

fprintf(stderr, "Filling hashtable...\n");
   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
//fprintf(stderr, "Filling with %s\n", entry->key.ascii());
      int hash = entry->hash % mHashTableSize;
      if (!hashTable[hash].entry)
      { // First entry
         hashTable[hash].entry = entry;
      }
      else 
      {
         if (!hashTable[hash].duplicates)
         { // Second entry, build duplicate list.
            hashTable[hash].duplicates = new QList<string_entry>();
            hashTable[hash].duplicates->append(hashTable[hash].entry);
            hashTable[hash].duplicate_offset = 0;
         }
         hashTable[hash].duplicates->append(entry);
      }
   }

   str << mHashTableSize;
   str << mHashList;

   mOffset = str.device()->at(); // mOffset points to start of hashTable
fprintf(stderr, "start of hash table at %08x\n", mOffset);

   for(int pass = 1; pass <= 2; pass++)
   {
      str.device()->at(mOffset);
fprintf(stderr, "Writing hash table (pass #%d)\n", pass);   
      for(uint i=0; i < mHashTableSize; i++)
      {
         if (!hashTable[i].entry)
            str << (Q_INT32) 0;
         else if (!hashTable[i].duplicates)
            str << (Q_INT32) hashTable[i].entry->payload->offset(); // Positive ID
         else
            str << (Q_INT32) -hashTable[i].duplicate_offset; // Negative ID
      }
fprintf(stderr, "End of Hash Table, offset = %d\n", str.device()->at());   

fprintf(stderr, "Writing duplicate lists (pass #%d)\n", pass);   
      for(uint i=0; i < mHashTableSize; i++)
      {
         if (hashTable[i].duplicates)
         {
            QList<string_entry> *dups = hashTable[i].duplicates;
            hashTable[i].duplicate_offset = str.device()->at();

//fprintf(stderr, "Duplicate list: Offset = %d list_size = %d\n",
//        hashTable[i].duplicate_offset, dups->count());

            for(string_entry *dup = dups->first(); dup; dup=dups->next())
            {
               str << (Q_INT32) dup->payload->offset(); // Positive ID
               str << dup->key;                         // Key (QString)
//fprintf(stderr, ">> %s\n", dup->key.ascii());
            }
            str << (Q_INT32) 0;               // End of list marker (0)
         }
      }
fprintf(stderr, "End of Dict, offset = %d\n", str.device()->at());   
   }

fprintf(stderr, "Cleaning up hash table.\n");
   for(uint i=0; i < mHashTableSize; i++)
   {
      delete hashTable[i].duplicates;
   }
   delete [] hashTable;

}

