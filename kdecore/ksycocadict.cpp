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

#include "ksycocadict.h"
#include "ksycocaentry.h"
#include "ksycoca.h"

#include <qptrlist.h>
#include <qvaluelist.h>
#include <kdebug.h>
#include <stdlib.h>

struct string_entry {
  string_entry(QString _key, KSycocaEntry *_payload) 
  { keyStr = _key; key = keyStr.unicode(); length = keyStr.length(); payload = _payload; hash = 0; }
  uint hash;
  int length;
  const QChar *key;
  QString keyStr;
  KSycocaEntry *payload;
};

template class QPtrList<string_entry>;

class KSycocaDictStringList : public QPtrList<string_entry>
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
   Q_UINT32 test1, test2;
   str->device()->at(offset);
   (*str) >> test1 >> test2;
   if ((test1 > 0x000fffff) || (test2 > 1024))
   {
       KSycoca::flagError();
       mHashTableSize = 0;
       mOffset = 0;
       return;
   }

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
   
int 
KSycocaDict::find_string(const QString &key )
{
   //kdDebug(7011) << QString("KSycocaDict::find_string(%1)").arg(key) << endl;

   if (!mStr || !mOffset)
   {
      kdError(7011) << "No database available!" << endl;
      return 0;
   }

   if (mHashTableSize == 0)
      return 0; // Unlikely to find anything :-]

   // Read hash-table data 
   uint hash = hashKey(key) % mHashTableSize;
   //kdDebug(7011) << QString("hash is %1").arg(hash) << endl;

   uint off = mOffset+sizeof(Q_INT32)*hash;
   //kdDebug(7011) << QString("off is %1").arg(off,8,16) << endl;
   mStr->device()->at( off );

   Q_INT32 offset;
   (*mStr) >> offset;

   //kdDebug(7011) << QString("offset is %1").arg(offset,8,16) << endl;
   if (offset == 0)
      return 0;

   if (offset > 0)
      return offset; // Positive ID

   // Lookup duplicate list.
   offset = -offset;

   mStr->device()->at(offset);
   //kdDebug(7011) << QString("Looking up duplicate list at %1").arg(offset,8,16) << endl;
   
   while(true)
   {
       (*mStr) >> offset;
       if (offset == 0) break;
       QString dupkey;
       (*mStr) >> dupkey;
       //kdDebug(7011) << QString(">> %1 %2").arg(offset,8,16).arg(dupkey) << endl;
       if (dupkey == key) return offset;
   }
   //kdWarning(7011) << "Not found!" << endl;

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
   register uint h = 0;
  
   for(uint i = 0; i < mHashList.count(); i++)
   {
      int pos = mHashList[i];
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
calcDiversity(KSycocaDictStringList *d, int pos, int sz)
{
   if (pos == 0) return 0;
   bool *matrix = (bool *) calloc(sz, sizeof(bool));
   uint usz = sz;

   if (pos < 0)
   {
      pos = -pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
	register int l = entry->length;
         if (pos < l && pos != 0)
         {
           register uint hash = ((entry->hash * 13) + (entry->key[l-pos].cell() % 29)) & 0x3ffffff;
	   matrix[ hash % usz ] = true;
         }
      }
   }
   else
   {
      pos = pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         if (pos < entry->length)
         {
            register uint hash = ((entry->hash * 13) + (entry->key[pos].cell() % 29)) & 0x3ffffff;
            matrix[ hash % usz ] = true;
         }
      }
   }
   int diversity = 0;
   for(int i=0;i< sz;i++)
      if (matrix[i]) diversity++;
   
   free((void *) matrix);

   return diversity;
}

//
// Add the diversity of the strings at position 'pos'
static void 
addDiversity(KSycocaDictStringList *d, int pos)
{
   if (pos == 0) return;
   if (pos < 0)
   {
      pos = -pos-1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
         register int l = entry->length;
         if (pos < l)
            entry->hash = ((entry->hash * 13) + (entry->key[l-pos].cell() % 29)) & 0x3fffffff;
      }
   }
   else
   {
      pos = pos - 1;
      for(string_entry *entry=d->first(); entry; entry = d->next())
      {
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
      mHashTableSize = 0;
      mHashList.clear();
      str << mHashTableSize;
      str << mHashList;
      return;
   }

   mOffset = str.device()->at();

   //kdDebug(7011) << QString("KSycocaDict: %1 entries.").arg(count()) << endl;

   //kdDebug(7011) << "Calculating hash keys.." << endl;

   int maxLength = 0;
   //kdDebug(7011) << "Finding maximum string length" << endl;
   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
      entry->hash = 0;
      if (entry->length > maxLength)
         maxLength = entry->length;
   }

   //kdDebug(7011) << QString("Max string length = %1").arg(maxLength) << endl;

   // use "almost prime" number for sz (to calculate diversity) and later
   // for the table size of big tables
   // int sz = d->count()*5-1;
   register unsigned int sz = count()*4 + 1;
   while(!(((sz % 3) && (sz % 5) && (sz % 7) && (sz % 11) && (sz % 13)))) sz+=2;

   int maxDiv = 0;
   int maxPos = 0;
   int lastDiv = 0;

   mHashList.clear();

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

         int diversity = calcDiversity(d, pos, sz);
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
      addDiversity(d, maxPos);
      mHashList.append(maxPos);
   }

   delete [] oldvec;

   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
      entry->hash = hashKey(entry->keyStr);
   }
// fprintf(stderr, "Calculating minimum table size..\n");

   mHashTableSize = sz;

   struct hashtable_entry {
      string_entry *entry;
      QPtrList<string_entry> *duplicates;
      int duplicate_offset;
   };

   hashtable_entry *hashTable = new hashtable_entry[ sz ];

   //kdDebug(7011) << "Clearing hashtable..." << endl;
   for (unsigned int i=0; i < sz; i++)
   {
      hashTable[i].entry = 0;
      hashTable[i].duplicates = 0;
   }

   //kdDebug(7011) << "Filling hashtable..." << endl;
   for(string_entry *entry=d->first(); entry; entry = d->next())
   {
      int hash = entry->hash % sz;
      if (!hashTable[hash].entry)
      { // First entry
         hashTable[hash].entry = entry;
      }
      else 
      {
         if (!hashTable[hash].duplicates)
         { // Second entry, build duplicate list.
            hashTable[hash].duplicates = new QPtrList<string_entry>();
            hashTable[hash].duplicates->append(hashTable[hash].entry);
            hashTable[hash].duplicate_offset = 0;
         }
         hashTable[hash].duplicates->append(entry);
      }
   }

   str << mHashTableSize;
   str << mHashList;

   mOffset = str.device()->at(); // mOffset points to start of hashTable
   //kdDebug(7011) << QString("Start of Hash Table, offset = %1").arg(mOffset,8,16) << endl;

   for(int pass = 1; pass <= 2; pass++)
   {
      str.device()->at(mOffset);
      //kdDebug(7011) << QString("Writing hash table (pass #%1)").arg(pass) << endl;
      for(uint i=0; i < mHashTableSize; i++)
      {
         Q_INT32 tmpid;
         if (!hashTable[i].entry)
            tmpid = (Q_INT32) 0;
         else if (!hashTable[i].duplicates)
            tmpid = (Q_INT32) hashTable[i].entry->payload->offset(); // Positive ID
         else
            tmpid = (Q_INT32) -hashTable[i].duplicate_offset; // Negative ID
         str << tmpid;
         //kdDebug(7011) << QString("Hash table : %1").arg(tmpid,8,16) << endl;
      }
      //kdDebug(7011) << QString("End of Hash Table, offset = %1").arg(str.device()->at(),8,16) << endl;

      //kdDebug(7011) << QString("Writing duplicate lists (pass #%1)").arg(pass) << endl;
      for(uint i=0; i < mHashTableSize; i++)
      {
         if (hashTable[i].duplicates)
         {
            QPtrList<string_entry> *dups = hashTable[i].duplicates;
            hashTable[i].duplicate_offset = str.device()->at();

            /*kdDebug(7011) << QString("Duplicate lists: Offset = %1 list_size = %2")                           .arg(hashTable[i].duplicate_offset,8,16).arg(dups->count()) << endl;
*/
            for(string_entry *dup = dups->first(); dup; dup=dups->next())
            {
               str << (Q_INT32) dup->payload->offset(); // Positive ID
               str << dup->keyStr;                      // Key (QString)
            }
            str << (Q_INT32) 0;               // End of list marker (0)
         }
      }
      //kdDebug(7011) << QString("End of Dict, offset = %1").arg(str.device()->at(),8,16) << endl;
   }

   //kdDebug(7011) << "Cleaning up hash table." << endl;
   for(uint i=0; i < mHashTableSize; i++)
   {
      delete hashTable[i].duplicates;
   }
   delete [] hashTable;
}

