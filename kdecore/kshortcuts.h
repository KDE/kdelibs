#ifndef _KSHORTCUTS_H
#define _KSHORTCUTS_H

#include <qstring.h>
#include <kkeysequence.h>

class KAccelShortcuts;

class KShortcuts
{
	friend class KAccel;
	friend class KGlobalAccel;
 public:
	KShortcuts();
	KShortcuts( const KShortcuts& );
	KShortcuts( const QString& s );
	KShortcuts( QKeySequence );
	KShortcuts( KKeySequence );
	~KShortcuts();
	
	KShortcuts& operator =( const KShortcuts& );

	KAccelShortcuts& base();
	operator const KAccelShortcuts&() const;

 private:
	class KShortcutsPrivate* d;
};

#endif // _KSHORTCUTS_H
