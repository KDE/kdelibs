#ifndef KURL_H
#define KURL_H

// -*-C++-*-
// KURL header
//
// Mon Nov 25 10:07:17 1996 -- Steffen Hansen

#include <qstring.h>
#include <qdir.h>

//Why the h... hasn't DCC on SGI true and false defined
//const bool true = 1;
//const bool false = 0;
#ifndef KURL_H
#define KURL_H

// -*-C++-*-
// KURL header
//
// Mon Nov 25 10:07:17 1996 -- Steffen Hansen

#include <qstring.h>
#include <qdir.h>

//Why the h... hasn't DCC on SGI true and false defined
//const bool true = 1;
//const bool false = 0;

/// KDE URL objects
/** The KURL class deals with uniform resource locators in a 
  protocol independent way. It works on {\tt file:}-type URL's
  much like QDir does on normal directories; but KURL extends 
  the directory operations to work on general URL's. In fact, the 
  part of KURL that only deals with syntax doesn't care about 
  the protocol at all, so feel free to use it to format any 
  URL-like string. NOTE: KURL doesn't support URL's that don't 
  look like files (for example {\tt mailto:someone@somewhere}).
  [If URL's like this were OK, there were noreason for 
  isMalformed() since any string with a {\tt :} would be a valid URL.
  Comments please.] 
  */

class KURL : private QDir { 
public:
  //@Man: Constructors
  //@{
  /// Construct a KURL object.
  /** Construct a KURL object.*/
  KURL() { malformed = true; protocol_part = ""; 
  host_part = ""; path_part = ""; ref_part = ""; } 
     
  /// Construct a KURL object from _url.
  /** Construct a KURL object from _url. 
	A KURL object is always constructed, but if you
	plan to use it, you should check it with isMalformed().
	*/
  KURL( const char* _url);
     
  /// Construct a KURL object from its components.
  /** Construct a KURL object from its components. */
  KURL( const char* _protocol, const char* _host, 
		const char* _path, const char* _ref);

  /// Constructs a URL.
  /**
	The second argument may be a relative URL, like '/home/weis/test.txt'.
	If for example the first parameter is 'http://uni-frankfurt/pub/incoming' then
	the result will be 'http://uni-frankfurt/home/weis/test.txt'. Of course the
	second argument may be a complete URL, too.
	*/
  KURL( KURL & _base_url, const char* _rel_url );
  //@}
     
  //@Man: URL information members
  //@{
  /// Tells wether the URL has the right syntax.
  /** Returns true if the URL is not a valid URL. This is only 
	syntax-checking -- it is not tested if the URL exists.
       
	{\bf NOTE:} Syntax checking is only done when constructing a 
	KURL from a string. 
	*/
  bool isMalformed() const { return malformed; }
     
  /// Returns the URL.
  /** Returns the URL. */
  QString url() const;
     
  /// Returns the protocol.
  /** The function returns the protocolname up to, but not
	including the {\tt :}.
	*/
  char* protocol() const { if (protocol_part.isNull()) return ""; 
  else return protocol_part.data(); }

  /// Returns the host.
  /** This function returns the host. If there is no host (i.e.
	the URL refers to a local file) this function returns "".
	*/
  char* host() const { if (host_part.isNull()) return "";  
  else return host_part.data(); }

  /// Returns the path.
  /** This function returns the path-part of the URL, for example 
	path() on {\tt tar://ftp.foo.org/bar/stuff.tar.gz\#tex/doc.tex}. 
	returns #/bar/stuff.tar.gz#.
	*/
  char* path() const { if (path_part.isNull()) return ""; 
  else return path_part.data(); }

  /// Returns the reference.
  /** This function returns the reference. If the URL is 
	{\tt http://www.nowhere/path/file.html\#toc}
	this function will return #toc#. If there is no 
	reference it returns "".
	*/
  char* reference() const { if (ref_part.isNull()) return ""; 
  else return ref_part.data(); }

  /// Returns the user name
  /**
    This function returns the user name or an empty string if
    no user has been specified.
    */
  const char* user() { if (user_part.isNull()) return ""; 
  else return user_part.data(); }

  /// Returns the password
  /**
    This function returns the password or an empty string if
    no password has been specified.
    */
  const char* passwd() { if (passwd_part.isNull()) return ""; 
  else return passwd_part.data(); }

  /// Returns the directory only
  /**
    If for example the URL is "file:/tmp/weis/file.html", then this call
    will return "/tmp/weis/". If you pass "file:/tmp/weis/" to this
    function, you will get "/tmp/weis/", because you already passed a directory.
    Turning the '_traling' flag off, causes the traling '/' to be ignored.
    "file:/tmp/weis/file.html" will result in "/tmp/weis/", too, but
    "file:/tmp/weis/" will lead to "/tmp/". As you see, this is
    a smart method to get the parent directory of a file/directory.
    This function is supplied for convenience only.
    */
  const char * directory( bool _trailing = TRUE );
  /// Returns the URL with the directory only
  /**
    If for example the URL is "file:/tmp/weis/file.html", then this call
    will return "file:/tmp/weis/". For more details look at 'directory(...)'
    */
  const char * directoryURL( bool _trailing = TRUE );
    
  //@}

  //@Man: URL modifying members
  //@{

  /// Parse a string
  void parse( const char *_url );
    
  /// Set protocol.
  /** Sets the protocol to newProto. Useful for example if an app hits
	{\tt file:/tmp/interesting.zip}, then it might do 
	setProtocol( "{\tt zip}").
	*/ 
  void setProtocol( const char* newProto) { protocol_part = newProto; }

  /// Set the password.
  void setPassword( const char *password ) { passwd_part = password; }     

  /// Set reference.
  /** Set reference. Convenience function to set the reference.
	A reference may be removed with setRef( ""). The function 
	returns false if it could not make a reference (if there were
	no path to reference from) and true on succes.
	*/
  bool setReference( const char* _ref);

  /// Clean up URL
  /** This function cleans up the URL, so that it doesn't contain
	{\tt /./}, {\tt /../} or  {\tt bla//bla} type stuff.
	*/
  void cleanURL();

  /// Change directory
  /** Changes directory by descending into the given directory. If 
	dir starts with a {\tt /} and acceptAbsPath is true the 
	current URL will be {\tt protocol://host/dir} otherwise dir will 
	be appended to the path, even if it starts with a {\tt /}. 
	If isReference is true, cd will make a reference unless 
	there already is one, then cd will append {\tt /dir} to the reference. 
	KURL u( {\tt tar:/tmp/file.tar.gz\#foo}); cd( "{\tt ..}"); is legal, 
	but it doesn't make much sense, since the result is {\tt tar:/tmp\#foo}.
	cd( "{\tt /}", true, true) will remove the reference if there is one.
       
	{\bf NOTE:} If the cd is done on the path-part of the URL, the 
	reference is lost -- this is normally desirable.
	*/   
  bool cd( const char* _dir, bool acceptAbsPath = true, 
		   bool isReference = false);

  /// Change directory path
  /** Change directory path. Same as cd with isReference = false. When
	zapRef is false, the reference is untouched by the 
	operation.
	*/
  bool cdPath( const char* _dir, bool acceptAbsPath = true, 
			   bool zapRef = true);

  /// Change reference path
  /** Change reference path. Same as cd with isReference = true */
  bool cdRef( const char* _ref, bool acceptAbsPath = true);

  /// Go to parent dir.
  /** Go to parent dir. If zapRef is true, the reference is removed, 
	otherwise it stays, but normally no one would want that. 
	*/
  bool cdUp( bool zapRef = true) { if( zapRef) setReference(""); 
  return cd( ".."); }

  /// Returns the filename or directory name of the URL
  /** If 'file:/home/weis/test.txt' is the URL, the result will be 'test.txt'
	If the URL us 'tar:/home/weis/test.tgz#foo/myfile' and isReference is TRUE,
	the function will return 'myfile'
	*/
  const char *filename( bool isReference = false );

  //@}
  //@Man: Operators
  //@{
  /// Copy
  /** Makes a copy of a URL
   */
  KURL &operator=( const KURL &);

  /// Initialize the URL with the given string.
  /**
	'_url' must be a valid URL.
	*/
  KURL &operator=( const char* _url );

  /// Compare URL's.
  /** Returns true if to URL's are equal, false otherwise.
   */
  bool operator==( const KURL &_url) const { return _url.url() == url(); }
  //@}
     
  //@Man:  Directory list members
  //@{
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QStrList* entryList(int filterSpec = DefaultFilter, 
							int sortSpec = DefaultSort) const;
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QStrList* entryList (const char* nameFilter, 
							 int filterSpec = DefaultFilter, 
							 int sortSpec = DefaultSort) const; 
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QFileInfoList* entryInfoList (int filterSpec = DefaultFilter, 
									  int sortSpec = DefaultSort) const; 
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QFileInfoList* entryInfoList (const char* nameFilter, 
									  int filterSpec = DefaultFilter, 
									  int sortSpec = DefaultSort) const; 
     
  //@}
protected:
  bool malformed;

  QString protocol_part;
  QString host_part;
  QString path_part;
  QString ref_part;
  /// This variable is only valid after calling 'directory'.
  QString dir_part;
  QString user_part;
  QString passwd_part;
    
private:
  void detach();
};

/* TODO: Implement entryList, entryInfoList, exists(), 
         isReadable(), isDir() and isFile() and and and ...
 */

#endif

/// KDE URL objects
/** The KURL class deals with uniform resource locators in a 
  protocol independent way. It works on {\tt file:}-type URL's
  much like QDir does on normal directories; but KURL extends 
  the directory operations to work on general URL's. In fact, the 
  part of KURL that only deals with syntax doesn't care about 
  the protocol at all, so feel free to use it to format any 
  URL-like string. NOTE: KURL doesn't support URL's that don't 
  look like files (for example {\tt mailto:someone@somewhere}).
  [If URL's like this were OK, there were noreason for 
  isMalformed() since any string with a {\tt :} would be a valid URL.
  Comments please.] 
  */

class KURL : private QDir { 
public:
  //@Man: Constructors
  //@{
  /// Construct a KURL object.
  /** Construct a KURL object.*/
  KURL() { malformed = true; protocol_part = ""; 
  host_part = ""; path_part = ""; ref_part = ""; } 
     
  /// Construct a KURL object from _url.
  /** Construct a KURL object from _url. 
	A KURL object is always constructed, but if you
	plan to use it, you should check it with isMalformed().
	*/
  KURL( const char* _url);
     
  /// Construct a KURL object from its components.
  /** Construct a KURL object from its components. */
  KURL( const char* _protocol, const char* _host, 
		const char* _path, const char* _ref);

  /// Constructs a URL.
  /**
	The second argument may be a relative URL, like '/home/weis/test.txt'.
	If for example the first parameter is 'http://uni-frankfurt/pub/incoming' then
	the result will be 'http://uni-frankfurt/home/weis/test.txt'. Of course the
	second argument may be a complete URL, too.
	*/
  KURL( KURL & _base_url, const char* _rel_url );
  //@}
     
  //@Man: URL information members
  //@{
  /// Tells wether the URL has the right syntax.
  /** Returns true if the URL is not a valid URL. This is only 
	syntax-checking -- it is not tested if the URL exists.
       
	{\bf NOTE:} Syntax checking is only done when constructing a 
	KURL from a string. 
	*/
  bool isMalformed() const { return malformed; }
     
  /// Returns the URL.
  /** Returns the URL. */
  QString url() const;
     
  /// Returns the protocol.
  /** The function returns the protocolname up to, but not
	including the {\tt :}.
	*/
  char* protocol() const { if (protocol_part.isNull()) return ""; 
  else return protocol_part.data(); }

  /// Returns the host.
  /** This function returns the host. If there is no host (i.e.
	the URL refers to a local file) this function returns "".
	*/
  char* host() const { if (host_part.isNull()) return "";  
  else return host_part.data(); }

  /// Returns the path.
  /** This function returns the path-part of the URL, for example 
	path() on {\tt tar://ftp.foo.org/bar/stuff.tar.gz\#tex/doc.tex}. 
	returns #/bar/stuff.tar.gz#.
	*/
  char* path() const { if (path_part.isNull()) return ""; 
  else return path_part.data(); }

  /// Returns the reference.
  /** This function returns the reference. If the URL is 
	{\tt http://www.nowhere/path/file.html\#toc}
	this function will return #toc#. If there is no 
	reference it returns "".
	*/
  char* reference() const { if (ref_part.isNull()) return ""; 
  else return ref_part.data(); }

  /// Returns the user name
  /**
    This function returns the user name or an empty string if
    no user has been specified.
    */
  const char* user() { if (user_part.isNull()) return ""; 
  else return user_part.data(); }

  /// Returns the password
  /**
    This function returns the password or an empty string if
    no password has been specified.
    */
  const char* passwd() { if (passwd_part.isNull()) return ""; 
  else return passwd_part.data(); }

  /// Returns the directory only
  /**
    If for example the URL is "file:/tmp/weis/file.html", then this call
    will return "/tmp/weis/". If you pass "file:/tmp/weis/" to this
    function, you will get "/tmp/weis/", because you already passed a directory.
    Turning the '_traling' flag off, causes the traling '/' to be ignored.
    "file:/tmp/weis/file.html" will result in "/tmp/weis/", too, but
    "file:/tmp/weis/" will lead to "/tmp/". As you see, this is
    a smart method to get the parent directory of a file/directory.
    This function is supplied for convenience only.
    */
  const char * directory( bool _trailing = TRUE );
  /// Returns the URL with the directory only
  /**
    If for example the URL is "file:/tmp/weis/file.html", then this call
    will return "file:/tmp/weis/". For more details look at 'directory(...)'
    */
  const char * directoryURL( bool _trailing = TRUE );
    
  //@}

  //@Man: URL modifying members
  //@{

  /// Parse a string
  void parse( const char *_url );
    
  /// Set protocol.
  /** Sets the protocol to newProto. Useful for example if an app hits
	{\tt file:/tmp/interesting.zip}, then it might do 
	setProtocol( "{\tt zip}").
	*/ 
  void setProtocol( const char* newProto) { protocol_part = newProto; }

  /// Set the password.
  void setPassword( const char *password ) { passwd_part = password; }     

  /// Set reference.
  /** Set reference. Convenience function to set the reference.
	A reference may be removed with setRef( ""). The function 
	returns false if it could not make a reference (if there were
	no path to reference from) and true on succes.
	*/
  bool setReference( const char* _ref);

  /// Clean up URL
  /** This function cleans up the URL, so that it doesn't contain
	{\tt /./}, {\tt /../} or  {\tt bla//bla} type stuff.
	*/
  void cleanURL();

  /// Change directory
  /** Changes directory by descending into the given directory. If 
	dir starts with a {\tt /} and acceptAbsPath is true the 
	current URL will be {\tt protocol://host/dir} otherwise dir will 
	be appended to the path, even if it starts with a {\tt /}. 
	If isReference is true, cd will make a reference unless 
	there already is one, then cd will append {\tt /dir} to the reference. 
	KURL u( {\tt tar:/tmp/file.tar.gz\#foo}); cd( "{\tt ..}"); is legal, 
	but it doesn't make much sense, since the result is {\tt tar:/tmp\#foo}.
	cd( "{\tt /}", true, true) will remove the reference if there is one.
       
	{\bf NOTE:} If the cd is done on the path-part of the URL, the 
	reference is lost -- this is normally desirable.
	*/   
  bool cd( const char* _dir, bool acceptAbsPath = true, 
		   bool isReference = false);

  /// Change directory path
  /** Change directory path. Same as cd with isReference = false. When
	zapRef is false, the reference is untouched by the 
	operation.
	*/
  bool cdPath( const char* _dir, bool acceptAbsPath = true, 
			   bool zapRef = true);

  /// Change reference path
  /** Change reference path. Same as cd with isReference = true */
  bool cdRef( const char* _ref, bool acceptAbsPath = true);

  /// Go to parent dir.
  /** Go to parent dir. If zapRef is true, the reference is removed, 
	otherwise it stays, but normally no one would want that. 
	*/
  bool cdUp( bool zapRef = true) { if( zapRef) setReference(""); 
  return cd( ".."); }

  /// Returns the filename or directory name of the URL
  /** If 'file:/home/weis/test.txt' is the URL, the result will be 'test.txt'
	If the URL us 'tar:/home/weis/test.tgz#foo/myfile' and isReference is TRUE,
	the function will return 'myfile'
	*/
  const char *filename( bool isReference = false );

  //@}
  //@Man: Operators
  //@{
  /// Copy
  /** Makes a copy of a URL
   */
  KURL &operator=( const KURL &);

  /// Initialize the URL with the given string.
  /**
	'_url' must be a valid URL.
	*/
  KURL &operator=( const char* _url );

  /// Compare URL's.
  /** Returns true if to URL's are equal, false otherwise.
   */
  bool operator==( const KURL &_url) const { return _url.url() == url(); }
  //@}
     
  //@Man:  Directory list members
  //@{
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QStrList* entryList(int filterSpec = DefaultFilter, 
							int sortSpec = DefaultSort) const;
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QStrList* entryList (const char* nameFilter, 
							 int filterSpec = DefaultFilter, 
							 int sortSpec = DefaultSort) const; 
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QFileInfoList* entryInfoList (int filterSpec = DefaultFilter, 
									  int sortSpec = DefaultSort) const; 
  /// Like in QDir.
  /** Like in QDir, but still unimplemented */
  const QFileInfoList* entryInfoList (const char* nameFilter, 
									  int filterSpec = DefaultFilter, 
									  int sortSpec = DefaultSort) const; 
     
  //@}
protected:
  bool malformed;

  QString protocol_part;
  QString host_part;
  QString path_part;
  QString ref_part;
  /// This variable is only valid after calling 'directory'.
  QString dir_part;
  QString user_part;
  QString passwd_part;
    
private:
  void detach();
};

/* TODO: Implement entryList, entryInfoList, exists(), 
         isReadable(), isDir() and isFile() and and and ...
 */

#endif
