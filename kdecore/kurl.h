#ifndef KURL_H
#define KURL_H

// -*-C++-*-
// KURL header
//
// Mon Nov 25 10:07:17 1996 -- Steffen Hansen

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qdir.h>

/** 
* A class for URL processing.
*
The KURL class deals with uniform resource locators in a 
* protocol independent way. It works on file:-type URLs
* much like QDir does on normal directories; but KURL extends 
* the directory operations to work on general URLs. In fact, the 
* part of KURL that only deals with syntax doesn't care about 
* the protocol at all, so feel free to use it to format any 
* URL-like string. 
* 
* NOTE: KURL doesn't support URL's that don't look like files 
* (for example mailto:someone@somewhere). [If URL's like this were OK, 
* there would be no reason for isMalformed() since any string with a 
* ":" would be a valid URL.  Comments please.] 
*
* @author Steffen Hansen (stefh@mip.ou.dk)
* @version $Id$
* @short A class for URL processing.
*/
class KURL : private QDir 
{ 
public:

/** 
* Construct a KURL object.
*/
  KURL();
     
/** 
* Construct a KURL object from _url. 
*
* A KURL object is always constructed, but if you plan to use it, 
* you should check it with isMalformed().
*/
  KURL( const char* _url);
     
/** 
* Construct a KURL object from its components. 
*/
  KURL( const char* _protocol, const char* _host, 
		const char* _path, const char* _ref);

/**
* Constructs a URL.
*
* The second argument may be a relative URL, like '/home/weis/test.txt'.
* If for example the first parameter is 'http://uni-frankfurt/pub/incoming' 
* then the result will be 'http://uni-frankfurt/home/weis/test.txt'. 
*
* Of course the second argument may be a complete URL, too.
*/
  KURL( KURL & _base_url, const char* _rel_url );
     
/** 
* Returns true if the URL is not a valid URL. This is only syntax-checking;
* whether the resource to which the URL points exists is not checked.
*       
* NOTE: Syntax checking is only done when constructing a KURL from a string. 
*/
  bool isMalformed() const { return malformed; }

/**
* Escapes some reserved characters within URLs (e.g. '#', '%').
*
* Some characters in filenames or directory names make troubles
* For example '#' or '%' makes problem, if they are interpreted
* and not ment to be interpreted. This why we must encode them.
* This functions encodes the given URL and returns a reference
* to the result for convidence.
*/
    static void encodeURL( QString& url );

/**
* Decodes escaped characters within URLs.
*
* This function looks for '%' within the URL and replaces this character 
* with hexcode of the next two characters. If the next characters are not 
* hex chararcters, 0 will be used and the character will be skipped.
*/
    static void decodeURL( QString& url );

/** 
* Returns the URL as a QString.
*/
  QString url() const;
     
/** 
* The function returns the protocolname up to, but not including the ":".
*/
  char* protocol() const { if (protocol_part.isNull()) return ""; 
  else return protocol_part.data(); }

/** 
* This function returns the host. If there is no host (i.e.
* the URL refers to a local file) this function returns "".
*/
  char* host() const;

/** 
* This function returns the path-part of the URL.
*
* For example, path() on "tar://ftp.foo.org/bar/stuff.tar.gz#tex/doc.tex" 
* returns "/bar/stuff.tar.gz".
*/
  char* path() const;

/** 
* This function returns the reference. 
*
* If the URL is "http://www.nowhere/path/file.html#toc", this function 
* will return "toc". If there is no reference it returns "".
* If we have some subprotocol in the URL like in 
* file:/tmp/kde.tgz#tar:/kfm.rpm#rpm:/doc/index.html#section
* then only the last reference is going to be returned, in this case
* "section". A URL like
* file:/tmp/kde.tgz#tar:/kfm.rpm#rpm:/doc/index.html
* would return "" since there is no reference. The stuff behind
* the '#' is a subprotocol!
*/
  char* reference() const;

/**
* This function returns the user name or an empty string if
* no user has been specified.
*/
  const char* user();

/**
* The password.
*
* @return the password, or an empty string if no password was specified.
*/
  const char* passwd();

/**
* The port number.
*
* @return the port number, or 0 if none was specified.
*/
    unsigned int port() const;
    
/**
* Returns the directory only.
*
* If for example the URL is "file:/tmp/weis/file.html", then this call
* will return "/tmp/weis/". If you pass "file:/tmp/weis/" to this
* function, you will get "/tmp/weis/", because you already passed a directory.
* Turning the '_traling' flag off, causes the traling '/' to be ignored.
* "file:/tmp/weis/file.html" will result in "/tmp/weis/", too, but
* "file:/tmp/weis/" will lead to "/tmp/". As you see, this is
* a smart method to get the parent directory of a file/directory.
*
* This function is supplied for convenience only.
*/
  const char * directory( bool _trailing = TRUE );

/**
* Returns the URL with the directory only.
*
* If for example the URL is "file:/tmp/weis/file.html", then this call
* will return "file:/tmp/weis/". For more details look at 'directory(...)'
*/
  const char * directoryURL( bool _trailing = TRUE );

    /**
     * @return TRUE if the URL has a sub protocol. For example file:/tmp/kde.tgz#tar:/kfm/main.cpp
     *         is a URL with subprotocol. Use this function to check wether some URL really
     *         references a complete file on your local hard disk and not some special data
     *         inside the file, like the example shows.
     */
    bool hasSubProtocol();

    /**
     * If the URL has no subprotocol, parentURL behaves like a call to @ref #url.
     * Otherwise the part of the URL left to the last subprotocol is returned.
     * For example file:/tmp/kde.tgz#tar:/kfm.rpm#rpm:/doc/index.html#section will return
     * file:/tmp/kde.tgz#tar:/kfm.rpm. As you can see, the last subprotocol is stripped.
     * If the original URL is for example file:/httpd/index.html#section
     * then exact this string is going to be returned.
     */
    QString parentURL();
    /**
     * This call returnes the other part of the URL, the part that is stripped by @ref #parentURL.
     * It returns always the right most subprotocol. If there is no subprotocol, the call
     * to this function returns an empty string. For example a URL 
     * file:/tmp/kde.tgz#tar:/kfm.rpm#rpm:/doc/index.html#section
     * would return rpm:/doc/index.html#section.
     */
    QString childURL();
    /**
     * This function behaves like @ref #childURL, but if there is no subprotocol,
     * this function returns the same @ref #url returns instead of an empty string.
     */
    QString nestedURL();

/**
* Parse a string.
*/
  void parse( const char *_url );
    
/** 
* Sets the protocol to newProto. Useful for example if an app hits
* "file:/tmp/interesting.zip", then it might do setProtocol( "zip").
*/ 
  void setProtocol( const char* newProto) { protocol_part = newProto; }

/**
* Set the password.
*/
  void setPassword( const char *password );

/** 
* Set reference. 
*
* A reference may be removed with setRef( ""). The function returns false 
* if it could not make a reference (if there were no path to reference 
* from) and true on succes.
*/
  bool setReference( const char* _ref);

/** 
* This function cleans up the URL, so that it doesn't contain "/./", 
* "/../" or  "bla//bla" type stuff.
	*/
  void cleanURL();

/** 
* Changes directory by descending into the given directory. 
* If dir starts with a "/" and acceptAbsPath is true the 
* current URL will be "protocol://host/dir" otherwise dir will 
* be appended to the path, even if it starts with a "/". 
* If isReference is true, cd will make a reference unless 
* there already is one, then cd will append "/dir" to the reference. 
* KURL u( tar:/tmp/file.tar.gz#foo}; cd( ".."); is legal, 
* but it doesn't make much sense, since the result is "tar:/tmp#foo".
* cd( "/", true, true) will remove the reference if there is one.
*       
* NOTE: If the cd is done on the path-part of the URL, the reference 
* is lost -- this is normally desirable.
*/   
  bool cd( const char* _dir, bool acceptAbsPath = true, 
		   bool isReference = false);

/** 
* Change directory path. Same as cd with isReference = false. When zapRef 
* is false, the reference is untouched by the operation.
*/
  bool cdPath( const char* _dir, bool acceptAbsPath = true, 
			   bool zapRef = true);

/** 
* Change reference path. Same as cd with isReference = true.
*/
  bool cdRef( const char* _ref, bool acceptAbsPath = true);

/** 
* Go to parent dir. If zapRef is true, the reference is removed, 
* otherwise it stays, but normally no one would want that. 
*/
  bool cdUp( bool zapRef = true);

/**
* Returns the filename or directory name of the URL.
*
* If 'file:/home/weis/test.txt' is the URL, the result will be 'test.txt'
* If the URL us 'tar:/home/weis/test.tgz#foo/myfile' and isReference is TRUE,
* the function will return 'myfile'
*/
  const char *filename( bool isReference = false );

/**
* Makes a copy of a URL.
*/
  KURL &operator=( const KURL &);

/** 
* Initialize the URL with the given string.
* '_url' must be a valid URL.
*/
  KURL &operator=( const char* _url );

/** 
* Compare URL's.
*
* @return true if the URLs are equal, false otherwise.
*/
  bool operator==( const KURL &_url) const ;
  //@}
     
/** 
* Like in QDir, but still unimplemented 
*/
  const QStrList* entryList(int filterSpec = DefaultFilter, 
			int sortSpec = DefaultSort) const;
/** 
* Like in QDir, but still unimplemented 
*/
  const QStrList* entryList (const char* nameFilter, 
			 int filterSpec = DefaultFilter, 
			 int sortSpec = DefaultSort) const; 
/** 
* Like in QDir, but still unimplemented 
*/
  const QFileInfoList* entryInfoList (int filterSpec = DefaultFilter, 
				  int sortSpec = DefaultSort) const; 

/** 
* Like in QDir, but still unimplemented 
*/
  const QFileInfoList* entryInfoList (const char* nameFilter, 
				  int filterSpec = DefaultFilter, 
				  int sortSpec = DefaultSort) const; 
     
protected:
    bool malformed;
    
    QString protocol_part;
    QString host_part;
    QString path_part;
    QString ref_part;
    // This variable is only valid after calling 'directory'.
    QString dir_part;
    QString user_part;
    QString passwd_part;
    int port_number;
    
private:
  void detach();
};

/* TODO: Implement entryList, entryInfoList, exists(), 
         isReadable(), isDir() and isFile() and and and ...
 */

#endif


