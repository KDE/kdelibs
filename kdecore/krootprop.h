#ifndef _KROOTPROP_H
#define _KROOTPROP_H_

#include <kapp.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <qdict.h>

/** 
* KDE desktop resources stored on the root window.
*
* A companion to the KConfig class
*
* The KRootProp class is used for reading and writing configuration entries
* to properties on the root window.
*
* All configuration entries are of the form "key=value".
*
* @see KConfig::KConfig
* @author Mark Donohoe (donohe@kde.org)
* @version 
* @short KDE Configuration Management class
*/
class KRootProp
{
private:	
  Display *kde_display;
  Window root;
  int screen;
  Atom at;
  QDict <QString> propDict;

protected:

public:
/** 
* Construct a KRootProp object. 
*
*/
   KRootProp();
   
/** 
* Destructor. 
*
* Writes back any dirty configuration entries.
*/
  ~KRootProp();

/** 
* Specify the property in which keys will be searched.
*
*/	
  void setProp(const QString& rProp="");

/**
* Read the value of an entry specified by rKey in the current property
*
* @param rKey	The key to search for.
* @param pDefault A default value returned if the key was not found.
* @return The value for this key or an empty string if no value
*	  was found.
*/	
  QString readEntry( const QString& rKey, 
  	  	  	  	  	  const char* pDefault = 0 ) const ;
					  
/**
* Read a numerical value. 
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it numerically.
*
* @param rKey The key to search for.
* @param nDefault A default value returned if the key was not found.
* @return The value for this key or 0 if no value was found.
*/
  int readNumEntry( const QString& rKey, int nDefault = 0 ) const;
  
/** 
* Read a QFont.
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it as a font object.
*
* @param rKey		The key to search for.
* @param pDefault	A default value returned if the key was not found.
* @return The value for this key or a default font if no value was found.
*/ 
  QFont readFontEntry( const QString& rKey, 
							  const QFont* pDefault = 0 ) const;

/** 
* Read a QColor.
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it as a color.
*
* @param rKey		The key to search for.
* @param pDefault	A default value returned if the key was not found.
* @return The value for this key or a default color if no value
* was found.
*/					  
  QColor readColorEntry( const QString& rKey,
								const QColor* pDefault = 0 ) const;
							  
	
/** 
* writeEntry() overridden to accept a const char * argument.
*
* This is stored to the current property when destroying the
* config object or when calling Sync().
*
* @param rKey		The key to write.
* @param rValue		The value to write.
* @return The old value for this key. If this key did not exist, 
*	  a NULL string is returned.	  
*
* @see #writeEntry
*/				
  QString writeEntry( const QString& rKey, const QString& rValue );
  
/** Write the key value pair.
* Same as above, but write a numerical value.
* @param rKey The key to write.
* @param nValue The value to write.
* @return The old value for this key. If this key did not
* exist, a NULL string is returned.	  
*/
  QString writeEntry( const QString& rKey, int nValue );
  
/** Write the key value pair.
* Same as above, but write a font
* @param rKey The key to write.
* @param rValue The value to write.
* @return The old value for this key. If this key did not
* exist, a NULL string is returned.	  
*/
  QString writeEntry( const QString& rKey, const QFont& rFont );
  
/** Write the key value pair.
* Same as above, but write a color
* @param rKey The key to write.
* @param rValue The value to write.
* @return The old value for this key. If this key did not
*  exist, a NULL string is returned.	  
*/
  QString writeEntry( const QString& rKey, const QColor& rColor );

/** Flush the entry cache.
* Write back dirty configuration entries to the current property,
*  This is called automatically from the destructor.
*/	
	void sync();
};

#endif
