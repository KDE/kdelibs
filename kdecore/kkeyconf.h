/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>

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
#ifndef _KKEYCONF_H
#define _KKEYCONF_H

#include <qdict.h>
#include <qaccel.h>

#include <kconfig.h>
#include <kkeydata.h>

/** Pointer to current KKeyConfig object. */
#define kKeys KKeyConfig::getKKeyConfig()

/** 
 * Create a KKeyConfig object. This should not be used in a KApplication.
 */
void initKKeyConfig( KConfig *pconf );

/** 
 * Return the keycode corresponding to the given string. The string must
 *  be something like "SHIFT+A" or "F1+CTRL+ALT" or "Backspace"
 *	for instance.
 *	Return 0 if the string is not recognized.
 */	
uint stringToKey(const QString& sKey );

/** 
 *  Return the string corresponding to the given keycode. 
 *  Return NULL if the keyCode is not recognized.
 */
const QString keyToString( uint keyCode );


/** 
 * The KKeyConfig class allows the easy management of "function/key"
 *  associations. It allows the user to configure (configurable) keys via
 *  the config file or via a dialog window.
 *
 *  There is three differents levels :
 *  \begin{itemize}
 * 	\item the "functionName" : it is just a string which represents an action
 *   which can be handle via different methods in different widgets. 
 *  \item the "keyCode" : it represents a key + modifiers (SHIFT, CTRL
 *  or/and ALT).
 *  \item the "widgetName" : each widget using keys should have such a name.
 *  \end{itemize}
 * 	 
 * How it works :
 *  \begin{itemize}
 *  \item Each "functionName" is associated with exactly one "keyCode" :
 *  addKey( functionName, keyCode )
 *  \item Each widget is registered with a "widgetName" :
 *  registerWidget( widgetName, this )
 *  \item For a given widget you can connect a "functionName/keyCode"
 *  association to a SLOT :
 *  connectFunction( widgetName, functionName, receiver, SLOT(...) )
 *  \item Then KKeyConfig manages automatically the keypress.
 * 	\end{itemize}
 *
 * @version $Id$
 * @short Allows easy management of function/accelerator association.
*/
class KKeyConfig 
{
 friend KKeyWidgetEntry;
 friend KKeyConfigure;
	
 public:
	/** 
	 * Construct a KKeyConfig object. 
	 * Do not use directly this constructor; use initKKeyConfig() instead. 
	 */
	KKeyConfig( KConfig* pConfig);
	
	/** 
	 * Call sync() and destroy the KKeyConfig object. 
	 */
	~KKeyConfig();

	/** 
	* Return the current KKeyConfig object. 
	*/
	static KKeyConfig * getKKeyConfig() { return pKKeyConfig; }

	/** 
	* Read the current key code associated with the function. 
	*/
	uint readCurrentKey( const QString& functionName );
	
	/** 
	* Read the default key code associated with the function. 
	*/
	uint readDefaultKey( const QString& functionName);
	
	/** 
	 * Add a "functionName/keyCode" association.
	 * 	 
	 * If "configurable" is set : search the KConfig object for a 
	 * value. If such a value exists, it is taken as the current key.
	 *
	 * NB : adding a preexisting function will remove the old association
	 *      and eventually the old connections.
	 * 	 
	 * @return FALSE if the keyCode is not correct.
	 */
	bool addKey( const QString& functionName, uint defaultKeyCode,
				 bool configurable = TRUE );
	
	/** 
	 * Add a "functionName/keyCode" association.
	 * 	 
	 * If "configurable" is set : search the KConfig object for a 
	 * value. If such a value exists, it is taken as the current key.
	 *
	 * NB : adding a preexisting function will remove the old association
	 *      and eventually the old connections.
	 * 	 
	 * @return FALSE if the keyCode is not correct.
	 */
	bool addKey( const QString& functionName, const QString& defaultKeyCode,
				 bool configurable = TRUE );
		
 	/** 
	 * Remove a "functionName/keyCode" association.
	 * 	 
	 * NB : it will remove the eventual connections
     */
    void removeKey( const QString& functionName );  
	
	/** 
	 * Register a widget for future connections. 
	 */
	void registerWidget( const QString& widgetName, QWidget* currentWidget );
	
	/** 
	 * Connect the functionName to a slot for a specific widget. 
	 */
	void connectFunction( const QString& widgetName, 
						  const QString& functionName,
						  const QObject* receiver, const char* member,
						  bool activate = TRUE );
	
	/** 
	 * Activate/disactivate a connection. 
	 */
	void toggleFunction( const QString& widgetName,
						 const QString& functionName, bool activate );
	
	/** 
	 * Remove the connection for a specific widget. 
	 */
	void disconnectFunction( const QString& widgetName,
							 const QString& functionName );

	/** 
	 * Disconnect all the functionName for a specific widget. 
	 */
	void disconnectAllFunctions( const QString& widgetName );
	
	/** 
	 * Write the current configurable associations in the KConfig object. 
	 */
	void sync();
	
	/** 
	 * Create a dialog showing all the associations and allow the user
	 * to configure the configurable ones. On OK it will use the new values,
	 * save them in the KConfig object and return TRUE.
	 * Nothing is change on CANCEL.
	 */
	bool configureKeys( QWidget *parent );
	
 private:
	KConfig *pConfig;
	int aAvailableId;
	QDict<KKeyEntry> aKeyDict;
	QDict<KKeyWidgetEntry> aWidgetDict;

	void KKeyConfig::internalDisconnectFunction( const QString& widgetName,
	  KKeyWidgetEntry *pWEntry, KKeyEntry *pEntry, KKeyConnectEntry *pCEntry );

 protected:
	static KKeyConfig *pKKeyConfig;  
	
	void destroyWidgetEntry( const QString& widgetName );
	void internalDisconnectAll( const QString& widgetName );
};

/** 
 * Must be used for one-button dialog with the functionName "Close dialog". 
 *  It registers the dialog widget and connect the function.
 */
#define D_CLOSE_KEY( dialogName, pDialog ) \
    { kKeys->registerWidget( dialogName, pDialog ); \
      kKeys->connectFunction( dialogName, "Close dialog", \
							  pDialog, SLOT(accept()) ); }
		  
/** 
 * Must be used for ok/cancel dialog with functionNames "Ok dialog"
 * and "Cancel dialog".
 * It registers the dialog and connect the function.
 */
#define D_OKCANCEL_KEY( dialogName, pDialog ) \
    { kKeys->registerWidget( dialogName, pDialog ); \
      kKeys->connectFunction( dialogName, "Ok dialog", \
                              pDialog, SLOT(accept()) ); \
      kKeys->connectFunction( dialogName, "Cancel dialog", \
                              pDialog, SLOT(reject()) ); }
	
#endif
