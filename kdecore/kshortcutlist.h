/*  This file is part of the KDE libraries
    Copyright (C) 2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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

#ifndef __KSHORTCUTLIST_H
#define __KSHORTCUTLIST_H

class QString;
class QVariant;
class KInstance;
class KShortcut;

/**********************************************************************
* This is a wrapper class which allows a function to use one interface
* to KActionCollection, KAccelActions, and KActionPtrList.
**********************************************************************/

class KShortcutList
{
 public:
	KShortcutList();
	virtual ~KShortcutList();

	virtual uint count() const = 0;
	virtual QString name( uint ) const = 0;
	virtual QString label( uint ) const = 0;
	virtual QString whatsThis( uint ) const = 0;
	virtual const KShortcut& shortcut( uint ) const = 0;
	virtual const KShortcut& shortcutDefault( uint ) const = 0;
	virtual bool isConfigurable( uint ) const = 0;
	virtual bool setShortcut( uint, const KShortcut& ) = 0;
	virtual bool isGlobal( uint ) const;

	virtual int index( const QString& sName ) const;
	virtual int index( const KKeySequence& ) const;
	virtual const KInstance* instance() const;

	// These are here in order to handle expansion.
	enum Other { };
	virtual QVariant getOther( Other, uint index ) const = 0;
	virtual bool setOther( Other, uint index, QVariant ) = 0;

	virtual bool save() const = 0;
	virtual bool readSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0 );
	virtual bool writeSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0,
			bool bWriteAll = false, bool bGlobal = false ) const;

 protected:
        virtual void virtual_hook( int id, void* data );
 private:
	class KShortcutListPrivate* d;
};

//---------------------------------------------------------------------
// KAccelShortcutList
//---------------------------------------------------------------------

class KAccel;
class KAccelActions;
class KGlobalAccel;

class KAccelShortcutList : public KShortcutList
{
 public:
	KAccelShortcutList( KAccel* );
	KAccelShortcutList( KGlobalAccel* );
	KAccelShortcutList( KAccelActions&, bool bGlobal );
	virtual ~KAccelShortcutList();

	virtual uint count() const;
	virtual QString name( uint i ) const;
	virtual QString label( uint ) const;
	virtual QString whatsThis( uint ) const;
	virtual const KShortcut& shortcut( uint ) const;
	virtual const KShortcut& shortcutDefault( uint ) const;
	virtual bool isConfigurable( uint ) const;
	virtual bool setShortcut( uint, const KShortcut& );
	virtual bool isGlobal( uint ) const;

	virtual QVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, QVariant );

	virtual bool save() const;

 protected:
	KAccelActions& m_actions;
	bool m_bGlobal;

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KAccelShortcutListPrivate* d;
};

namespace KStdAccel {
//---------------------------------------------------------------------
// ShortcutList
//---------------------------------------------------------------------

class ShortcutList : public KShortcutList
{
 public:
	ShortcutList();
	virtual ~ShortcutList();

	virtual uint count() const;
	virtual QString name( uint i ) const;
	virtual QString label( uint ) const;
	virtual QString whatsThis( uint ) const;
	virtual const KShortcut& shortcut( uint ) const;
	virtual const KShortcut& shortcutDefault( uint ) const;
	virtual bool isConfigurable( uint ) const;
	virtual bool setShortcut( uint, const KShortcut& );

	virtual QVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, QVariant );

	virtual bool save() const;

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class ShortcutListPrivate* d;
};
};

#endif // __KSHORTCUTLIST_H
