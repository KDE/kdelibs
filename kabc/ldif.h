/*
    This file is part of libkabc.
    Copyright (c) 2004 Szombathelyi Gyorgy <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _K_LDIF_H_
#define _K_LDIF_H_

#include <qstring.h>
#include <qcstring.h>
#include <qmemarray.h>

namespace KABC {

  /**
   * LDIF
 
   * LDIF implements an RFC 2849 compliant LDIF parser. LDIF files are used to
   * represent directory information on LDAP-based servers, or to describe a set
   * of changes which are to be applied to a directory. 
   */

  class LDIF 
  {
  public:

    typedef enum ParseVal{ None, NewEntry, EndEntry, Item, Control, Err, MoreData };
    typedef enum EntryType{ Entry_None, Entry_Add, Entry_Del, Entry_Mod, Entry_Modrdn };
    typedef enum ModType{ Mod_None, Mod_Add, Mod_Replace, Mod_Del };
    LDIF();
    virtual ~LDIF();
    
    /**
     * Assembles fieldname and value into a valid LDIF line, BASE64 encodes the 
     * value if neccessary and optionally splits into more lines.
     */
    static QCString assembleLine( const QString &fieldname, 
      const QByteArray &value, uint linelen=0, bool url=false );
    
    static QCString assembleLine( const QString &fieldname, 
      const QCString &value, uint linelen=0, bool url=false );
    
    /**
     * Splits one line from an LDIF file to attribute and value components.
     * @returns true if value is an URL, false otherwise
     */
    static bool splitLine( const QCString &line, QString &fieldname, QByteArray &value );
    /**
     * Starts the parsing of a new LDIF
     */
    void startParsing();
    /**
     * Process one LDIF line
     */
    ParseVal processLine();
    /**
     * Process the LDIF until a complete item can be returned
     * @returns NewEntry if a new DN encountered, 
     * Item if a new item returned, 
     * Err if the LDIF contains error,
     * EndEntry if the parser reached the end of the current entry
     * and MoreData if the parser encountered the end of the current chunk of 
     * the LDIF. If you want to finish the parsing after receiving 
     * MoreData, then supply a buffer with '\\n\\n\\n' with setLDIF, so  the 
     * parser can safely flush the current entry.
     */
    ParseVal nextItem();
    /**
     * Sets a chunk of LDIF. Call before startParsing(), or if nextItem() returned
     * MoreData.
     */
    void setLDIF(QByteArray ldif) { mLdif = ldif; mPos = 0; };
    /**
     * Returns the requested LDAP operation extracted from the current entry.
     */
    EntryType entryType() { return mEntryType; };
    /**
     * Returns the LDAP modify request type if entryType() returned Entry_Mod.
     */
    int modType() { return mModType; };
    /**
     * Returns the Distinguished Name of the current entry.
     */
    QString Dn() { return mDn; };
    /**
     * Returns the new Relative Distinguished Name if modType() returned Entry_Modrdn.
     */
    QString newRdn() { return mNewRdn; };
    /**
     * Returns the new parent of the entry if modType() returned Entry_Modrdn.
     */
    QString newSuperior() { return mNewSuperior; };
    /**
     * Returns if the delete of the old RDN is required.
     */
    bool delOldRdn() { return mDelOldRdn; };
    /**
     * Returns the attribute name.
     */
    QString attr() { return mAttr; };
    /**
     * Returns the attribute value.
     */
    QByteArray val() { return mVal; };
    /**
     * Returns the criticality level when modType() returned Control.
     */
    bool critical() { return mCritical; };
    /**
     * Returns the OID when modType() returned Control.
     */
    QString oid() { return mOid; };
    /**
     * Returns the line number which the parser processes.
     */
    uint lineNo() { return mLineNo; };
  private:
    int mModType;
    bool mDelOldRdn;
    QString mDn,mAttr,mNewRdn,mNewSuperior, mOid;
    QByteArray mLdif, mVal;
    EntryType mEntryType;
    
    bool mIsNewLine, mIsComment,mCritical;
    ParseVal mLastParseVal;
    uint mPos,mLineNo;  
    QCString line;
        
    class LDIFPrivate;
    LDIFPrivate *d;
  };
}

#endif
