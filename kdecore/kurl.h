/* This file is part of the KDE libraries
    Copyright (C) 1997 Steffen Hansen (stefh@dit.ou.dk)

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
#ifndef KURL_H
#define KURL_H

// -*-C++-*-
// KURL header
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qstrlist.h>

/** 
* A class for URL processing.
*
* The KURL class deals with uniform resource locators in a 
* protocol independent way. It works on file:-type URLs
* much like @ref QDir does on normal directories; but KURL extends 
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
* First version by Torben Weis, redesigned by Steffen Hansen (stefh@mip.ou.dk),
* maintained by Torben Weis (weis@kde.org). Endcoding/Decoding done by
* Stephan Kulow (coolo@kde.org).
*
* @author Torben Weis (weis@kde.org)
*
* @version $Id$
* @short A class for URL processing.
*/

class KURL
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
     *
     * if the parameter is an absolute filename, it adds a file: prefix 
     * and encodes the path.
     */
    KURL( const char* _url);

    ~KURL();

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
    const char* protocol() const;
    
    /** 
     * This function returns the host. If there is no host (i.e.
     * the URL refers to a local file) this function returns "".
     */
    const char* host() const;
    
    /** 
     * This function returns the path-part of the URL.
     *
     * For example, path() on "tar://ftp.foo.org/bar/stuff.tar.gz#tex/doc.tex" 
     * returns "/bar/stuff.tar.gz".
     */
    const char* path() const;
    
    /** 
     * This function returns the path-part of an URL and leaves it as is.
     *
     * For example, path() on "http://www.foo.org/bar/cgi%2Dbin?value=blue%3f" 
     * returns "/bar/cgi%2Dbin".
     */
    const char* httpPath() const;
    
    /**
     * If we parse for example ftp://weis@localhost then we dont have a path.
     * The URL means: enter the home directory of user weis, while
     * ftp://weis@localhost/ means, login as user weis and enter the
     * root directory. KURL returns "/" as path in both cases. This function
     * lets you distinguish both URLs. It returns true in the first case.
     *
     * @see #bNoPath
     */
    bool hasPath() const { return !bNoPath; }
    
    /**
     * The search-part.
     *
     * @return the search-part, or NULL if no search-part was specified.
     */
    const char* searchPart() const;

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
    const char* reference() const;
    
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
     * Turning the '_trailing' flag off, causes the trailing '/' to be ignored.
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
    void setProtocol( const char* newProto) ;
    
    /**
     * Set the password.
     */
    void setPassword( const char *password );

    /**
     * Set the search-part.
     *
     * The search part of an URL is the part behind the '?'.
     * Example the URL "http://www.yahoo.com/search.cgi?country=netherland"
     * has "country=netherland" as search-part
     *
     * If NULL is specified, the search-part is cleared.
     */
    void setSearchPart( const char *_searchPart );
    
    /** 
     * Set reference. 
     *
     * A reference may be removed with setRef( ""). The function returns false 
     * if it could not make a reference (if there were no path to reference 
     * from) and true on succes.
     */
    bool setReference( const char* _ref);

    /** 
     * Changes directory by descending into the given directory. 
     * If dir starts with a "/" the 
     * current URL will be "protocol://host/dir" otherwise dir will 
     * be appended to the path.
     * If 'zapRef' is true, the reference will be deleted.
     */   
    bool cd( const char* _dir, bool zapRef = true);
    
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
    const char *filename();
    
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


    /**
      * Checks, if the URL refers to a usual file, that
      * can be openend with usual methods.
      *
      * Note: It doesn't check, if the file exist
      *
      * @return true, if the URL is a file, that can be opened
      **/
    bool isLocalFile();

protected:
    void cleanPath();
    
    bool malformed;
    /**
     * If we parse for example ftp://weis@localhost then we dont have a path.
     * The URL means: enter the home directory of user weis, while
     * ftp://weis@localhost/ means, login as user weis and enter the
     * root directory. KURL returns "/" as path in both cases. This variable
     * is used to distinguish both URLs. It is true in the first case.
     *
     * @see #hasPath
     */
    bool bNoPath;
    int port_number; 

    QString protocol_part;
    QString host_part;
    QString path_part;
    QString path_part_decoded;
	 QString search_part;
    QString ref_part;
    // This variable is only valid after calling 'directory'.
    QString dir_part;
    QString user_part;
    QString passwd_part;
    
private:
    void detach();
};



#endif


