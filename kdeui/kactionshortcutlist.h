#ifndef _KACTIONSHORTCUTLIST_H
#define _KACTIONSHORTCUTLIST_H

#include <kshortcutlist.h>

class KActionCollection;
class KActionPtrList;

//---------------------------------------------------------------------
// class KActionShortcutList
//---------------------------------------------------------------------

class KAccelShortcutListPrivate;
class KActionShortcutList : public KShortcutList
{
 public:
	KActionShortcutList( KActionCollection* );
	virtual ~KActionShortcutList();

	virtual uint count() const;
	virtual QString name( uint i ) const;
	virtual QString label( uint ) const;
	virtual QString whatsThis( uint ) const;
	virtual const KShortcut& shortcut( uint ) const;
	virtual const KShortcut& shortcutDefault( uint ) const;
	virtual bool isConfigurable( uint ) const;
	virtual bool setShortcut( uint, const KShortcut& );

	virtual const KInstance* instance() const;

	virtual QVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, QVariant );

	virtual bool save() const;

 protected:
	KActionCollection& m_actions;

 protected:
        virtual void virtual_hook( int id, void* data );
 private:
	KAccelShortcutListPrivate* d;
};

//---------------------------------------------------------------------
// class KActionPtrShortcutList
//---------------------------------------------------------------------

class KAccelShortcutListPrivate;
class KActionPtrShortcutList : public KShortcutList
{
 public:
	KActionPtrShortcutList( KActionPtrList& );
	virtual ~KActionPtrShortcutList();

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
	KActionPtrList& m_actions;

 protected:
       virtual void virtual_hook( int id, void* data );
 private:
	KAccelShortcutListPrivate* d;
};

#endif // !_KACTIONSHORTCUTLIST_H
