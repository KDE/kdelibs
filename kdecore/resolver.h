/*  -*- mode: C++; coding: utf-8; -*-
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License.
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
 */

#ifndef KRESOLVER_H
#define KRESOLVER_H

//////////////////
// Needed includes
#include "ksockaddr.h"
#include <qvaluelist.h>

////////////////////////
// Forward declarations
struct sockaddr;
class QString;
class QCString;
class QStrList;

//////////////////
// Our definitions

namespace KDE 
{ 
  namespace Network
  {
    class ResolverEntryPrivate;
    /**
     * One resolution entry
     *
     * This class is one element in the resolution results list.
     * It contains the socket address for connecting, as well as
     * a bit more of information: the socket type, address family
     * and protocol numbers.
     *
     * This class contains all the information required for creating,
     * binding and connecting a socket.
     *
     * ResolverEntry objects implicitly share data, so copying them
     * is quite efficient.
     *
     * @author Thiago Macieira <thiagom@mail.com>
     * @version $Id$
     * @short resolution entry
     */
    class ResolverEntry
    {
    public:
      /**
       * Default constructor
       *
       */
      ResolverEntry();

      /**
       * Constructs a new ResolverEntry from a KSocketAddress
       * and other data.
       *
       * The KSocketAddress @p addr parameter will be deep-copied.
       */
      ResolverEntry(const KSocketAddress* addr, int socktype, int protocol,
		    const QString& canonName = QString::null,
		    const QCString& encodedName = QCString());

      /**
       * Constructs a new ResolverEntry from raw forms of
       * socket addresses and other data.
       *
       * This constructor instead creates an internal KSocketAddress object.
       *
       * @param sa	the sockaddr structure containing the raw address
       * @param salen	the length of the sockaddr structure
       */
      ResolverEntry(const struct sockaddr *sa, ksocklen_t salen, int socktype, 
		    int protocol, const QString& canonName = QString::null,
		    const QCString& encodedName = QCString());

      /**
       * Copy constructor.
       *
       * This constructor performs a shallow-copy of the other object.
       */
      ResolverEntry(const ResolverEntry &other);

      /**
       * Destructor.
       *
       * The destructor frees associated resources with this object. It does
       * not destroy shared data.
       */
      virtual ~ResolverEntry();

      /**
       * Retrieves the socket address associated with this entry.
       */
      const KSocketAddress* address() const;

      /**
       * Retrieves the length of the socket address structure.
       */
      ksocklen_t length() const;

      /**
       * Retrieves the family associated with this socket address.
       */
      int family() const;

      /**
       * Retrieves the canonical name associated with this entry, if there is any.
       * If the canonical name was not found, this function returns QString::null.
       */
      QString canonicalName() const;

      /**
       * Retrieves the encoded domain name associated with this entry, if there is
       * any. If this domain has been resolved through DNS, this will be the
       * the ACE-encoded hostname.
       *
       * Returns a null QCString if such information is not available.
       *
       * Please note that this information is NOT to be presented to the user,
       * unless requested.
       */
      QCString encodedName() const;

      /**
       * Retrieves the socket type associated with this entry.
       */
      int socketType() const;

      /**
       * Retrieves the protocol associated with this entry.
       */
      int protocol() const;

      /**
       * Assignment operator
       *
       * This function copies the contents of the other object into this one.
       * Data will be shared between the two of them.
       */
      ResolverEntry& operator=(const ResolverEntry& other);

    private:
      ResolverEntryPrivate* d;
    };

    class ResolverResultsPrivate;
    /**
     * Name and service resolution results.
     *
     * This object contains the results of a name and service resolution, as
     * those performed by @ref Resolver. It is also a descendant of QValueList, so
     * you may use all its member functions here to access the elements.
     *
     * A ResolverResults object is associated with a resolution, so, in addition
     * to the resolved elements, you can also retrieve information about the 
     * resolution process itself, like the nodename that was resolved or an error 
     * code.
     *
     * Note Resolver also uses ResolverResults objects to indicate failure, so
     * you should test for failure.
     *
     * @author Thiago Macieira <thiagom@mail.com>
     * @version $Id$
     * @short name resolution results
     */
    class ResolverResults: public QValueList<ResolverEntry>
    {
    public:
      /**
       * Default constructor.
       *
       * Constructs an empty list.
       */
      ResolverResults();

      /**
       * Copy constructor
       *
       * Creates a new object with the contents of the other one. Data will be
       * shared by the two objects, like QValueList
       */
      ResolverResults(const ResolverResults& other);

      /**
       * Destructor
       *
       * Destroys the object and frees associated resources.
       */
      virtual ~ResolverResults();

      /**
       * Assignment operator
       *
       * Copies the contents of the other container into this one, discarding
       * our current values.
       */
      ResolverResults& operator=(const ResolverResults& other);

      /**
       * Retrieves the error code associated with this resolution. The values
       * here are the same as in @ref Resolver::ErrorCodes.
       */
      int errorCode() const;

      /**
       * Retrieves the system error code, if any.
       * @see Resolver::systemError for more information
       */
      int systemError() const;

      /**
       * Sets the error codes
       *
       * @param errorcode		the error code in @ref Resolver::ErrorCodes
       * @param systemerror	the system error code associated, if any
       */
      void setError(int errorcode, int systemerror = 0);

      /**
       * The nodename to which the resolution was performed.
       */
      QString nodeName() const;

      /**
       * The service name to which the resolution was performed.
       */
      QString serviceName() const;

      /**
       * Sets the new nodename and service name
       */
      void setAddress(const QString& host, const QString& service);

    protected:
      /// @internal
      virtual void virtual_hook( int id, void* data );
    private:
      ResolverResultsPrivate* d;
    };

    class ResolverPrivate;
    /**
     * Name and service resolution class.
     *
     * This class provides support for doing name-to-binary resolution
     * for nodenames and service ports. You should use this class if you
     * need specific resolution techniques when creating a socket or if you
     * want to inspect the results before calling the socket functions.
     *
     * You can either create an object and set the options you want in it
     * or you can simply call the static member functions, which will create
     * standard Resolver objects and dispatch the resolution for you. Normally,
     * the static functions will be used, except in cases where specific options
     * must be set.
     *
     * A Resolver object defaults to the following:
     * @li address family: any address family
     * @li socket type: streaming socket
     * @li protocol: implementation-defined. Generally, TCP
     * @li host and service: unset
     *
     * @author Thiago Macieira <thiagom@mail.com>
     * @version $Id$
     * @short name resolution
     */
    class Resolver: public QObject
    {
      Q_OBJECT

    public:

      /**
       * Address family selection types
       *
       * These values can be OR-ed together to form a composite family selection.
       *
       * @li UnknownFamily: a family that is unknown to the current implementation
       * @li KnownFamily: a family that is known to the implementation (the exact
       *		opposite of UnknownFamily)
       * @li AnyFamilies: any address family is acceptable
       * @li InternetFamily: an address for connecting to the Internet
       * @li InetFamily: alias for InternetFamily
       * @li IPv6Family: an IPv6 address only
       * @li IPv4Family: an IPv4 address only
       * @li UnixFamily: an address for the local Unix namespace (i.e., Unix sockets)
       * @li LocalFamily: alias for UnixFamily
       */
      enum SocketFamilies
      {
	UnknownFamily = 0x0001,

	UnixFamily = 0x0002,
	LocalFamily = UnixFamily,

	IPv4Family = 0x0004,
	IPv6Family = 0x0008,
	InternetFamily = IPv4Family | IPv6Family,
	InetFamily = InternetFamily,

	KnownFamily = ~UnknownFamily,
	AnyFamily = KnownFamily | UnknownFamily
      };

      /**
       * Flags for the resolution.
       *
       * These flags are used for setting the resolution behaviour for this
       * object:
       * @li Passive: resolve to a passive socket (i.e., one that can be used for
       *		binding to a local interface)
       * @li CanonName: request that the canonical name for the given nodename 
       *		be found and recorded)
       * @li NoResolve: request that no external resolution be performed. The given
       *		nodename and servicename will be resolved locally only.
       * @li NoSrv: don't try to use SRV-based name-resolution.
       */
      enum Flags
	{
	  Passive = 0x01,
	  CanonName = 0x02,
	  NoResolve = 0x04,
	  NoSrv = 0x08
	};

      /**
       * Flags for the reverse resolution.
       *
       * These flags are used by the @ref reverseResolve static functions for
       * setting resolution parameters. The possible values are:
       * @li NumericHost: don't try to resolve the host address to a text form.
       *		Instead, convert the address to its numeric textual representation.
       * @li NumericService: the same as NumericHost, but for the service name
       * @li NodeNameOnly: returns the node name only (i.e., not the Fully 
       *		Qualified Domain Name)
       * @li Datagram: in case of ambiguity in the service name, prefer the
       *		name associated with the datagram protocol
       */
      enum ReverseFlags
	{
	  NumericHost = 0x01,
	  NumericService = 0x02,
	  NodeNameOnly = 0x04,
	  Datagram = 0x08
	};

      /**
       * Error codes
       *
       * These are the possible error values that objects of this class
       * may return. See @ref strError for getting a string representation
       * for these errors.
       *
       * @li AddrFamily: Address family for the given nodename is not supported.
       * @li TryAgain: Temporary failure in name resolution. You should try again.
       * @li Failure: Non-recoverable failure in name resolution.
       * @li BadFlags: Invalid flags were given.
       * @li Memory: Memory allocation failure.
       * @li NoData: No address of the given kind associated with the requested name.
       *		This generally happens the address exists, but no address of the
       *		requested kind is available.
       * @li NoName: The specified name or service doesn't exist.
       * @li UnsupportedFamily: The requested socket family is not supported.
       * @li UnsupportedService: The requested service is not supported for this
       *		socket type (i.e., a datagram service in a streaming socket).
       * @li UnsupportedSocketType: The requested socket type is not supported.
       * @li SystemError: A system error occurred. See @ref sysError.
       * @li Canceled: This request was cancelled by the user.
       */
      enum ErrorCodes
	{
	  // note: if you change this enum, take a look at Resolver::strError
	  NoError = 0,
	  AddrFamily = -1,
	  TryAgain = -2,
	  Failure = -3,
	  BadFlags = -4,
	  Memory = -5,
	  NoData = -6,
	  NoName = -7,
	  UnsupportedFamily = -7,
	  UnsupportedService = -8,
	  UnsupportedSocketType = -9,
	  SystemError = -10,
	  Canceled = -100
	};

      /**
       * Status codes.
       *
       * These are the possible status for a Resolver object. A value
       * greater than zero indicates normal behaviour, while negative
       * values either indicate failure or error.
       *
       * @li Idle: resolution has not yet been started.
       * @li Queued: resolution is queued but not yet in progress.
       * @li InProgress: resolution is in progress.
       * @li Success: resolution is done; you can retrieve the results.
       * @li Canceled: request cancelled by the user.
       * @li Failed: resolution is done, but failed.
       *
       * Note: while the status code Canceled here is the same as the
       * error code Canceled above, be careful not to mistake the
       * status Failed with the above error code Failure. See @ref ErrorCodes.
       *
       * Note 2: the status Queued and InProgress might not be distinguishable.
       * Some implementations might not differentiate one from the other.
       */
      enum StatusCodes
	{
	  Idle = 0,
	  Queued = 1,
	  InProgress = 2,
	  Success = 3,
	  //Canceled = -100,	// already defined above
	  Failed = -101
	};


      /**
       * Default constructor.
       *
       * Creates an empty Resolver object. You should set the wanted
       * names and flags using the member functions before starting
       * the name resolution.
       */
      Resolver(QObject *parent = 0L, const char * name = 0L);

      /**
       * Constructor with host and service names.
       *
       * Creates a Resolver object with the given host and
       * service names. Flags are initialised to 0 and any address family
       * will be accepted.
       *
       * @param nodename	The host name we want resolved.
       * @param servicename	The service name associated, like "http".
       */
      Resolver(const QString& nodename, const QString& servicename = QString::null,
	       QObject * parent = 0L, const char *name = 0L);

      /**
       * Destructor.
       *
       * When this object is deleted, it'll destroy all associated
       * resources. If the resolution is still in progress, it will be
       * cancelled.
       */
      virtual ~Resolver();

      /**
       * Retrieve the current status of this object.
       *
       * @see StatusCodes for the possible status codes.
       */
      int status() const;

      /**
       * Retrieve the error code in this object.
       *
       * This function will return NoError if we are not in
       * an error condition. See @ref status and @ref StatusCodes to
       * find out what the current status is.
       *
       * @see strError for getting a textual representation of
       * this error
       */
      int errorCode() const;

      /**
       * Retrieve the associated system error code in this object.
       *
       * Many resolution operations may generate an extra error code
       * as given by the C errno variable. That value is stored in the
       * object and can be retrieved by this function.
       */
      int systemError() const;

      /**
       * Returns true if this object is currently running
       */
      bool isRunning() const;

      /**
       * The nodename to which the resolution was/is to be performed.
       */
      QString nodeName() const;

      /**
       * The service name to which the resolution was/is to be performed.
       */
      QString serviceName() const;

      /**
       * Sets the nodename for the resolution.
       *
       * Set the nodename to QString::null to unset it.
       * @param nodename		The nodename to be resolved.
       */
      void setNode(const QString& nodename);

      /**
       * Sets the service name to be resolved.
       *
       * Set it to QString::null to unset it.
       * @param service		The service to be resolved.
       */
      void setService(const QString& service);

      /**
       * Sets both the host and the service names.
       *
       * Setting either value to QString::null will unset them.
       * @param node		The nodename
       * @param service		The service name
       */
      void setAddress(const QString& node, const QString& service);

      /**
       * Retrieves the flags set for the resolution.
       *
       * @see Flags for an explanation on what flags are possible
       */
      int flags() const;

      /**
       * Sets the flags.
       *
       * @param flags		the new flags
       * @return			the old flags
       * @see Flags for an explanation on the flags
       */
      int setFlags(int flags);

      /**
       * Sets the allowed socket families.
       *
       * @param families		the families that we want/accept
       * @see SocketFamilies for possible values
       */
      void setFamily(int families);

      /**
       * Sets the socket type we want.
       *
       * The values for the @p type parameter are the SOCK_*
       * constants, defined in <sys/socket.h>. The most common
       * values are:
       *  @li SOCK_STREAM		streaming socket (= reliable, sequenced,
       *				connection-based)
       *  @li SOCK_DGRAM		datagram socket (= unreliable, connectionless)
       *  @li SOCK_RAW		raw socket, with direct access to the
       *				container protocol (such as IP)
       *
       * These three are the only values to which it is guaranteed that
       * resolution will work. Some systems may define other constants (such as
       * SOCK_RDM for reliable datagrams), but support is implementation-defined.
       * 
       * @param type		the wanted socket type (SOCK_* constants). Set
       *				0 to use the default.
       */
      void setSocketType(int type);

      /**
       * Sets the protocol we want.
       *
       * Protocols are dependant on the selected address family, so you should know
       * what you are doing if you use this function. Besides, protocols generally
       * are either stream-based or datagram-based, so the value of the socket
       * type is also important. The resolution will fail if these values don't match.
       *
       * When using an Internet socket, the values for the protocol are the
       * IPPROTO_* constants, defined in <netinet/in.h>.
       *
       * You may choose to set the protocol either by its number or by its name, or
       * by both. If you set:
       * @li the number and the name: both values will be stored internally; you
       *		may set the name to an empty value, if wanted
       * @li the number only (name = NULL): the name will be searched in the 
       *		protocols database
       * @li the name only (number = 0): the number will be searched in the
       *		database
       * @li neither name nor number: reset to default behaviour
       *
       * @param protonum		the protocol number we want
       * @param protoname		the protocol name
       */
      void setProtocol(int protonum, const char *name = 0L);

      /**
       * Starts the name resolution synchronously.
       *
       * This function will start the name resolution and wait
       * for its conclusion. Upon exit, this object will either
       * have succeeded in resolution (status Success) or will
       * have failed (status Failed or Canceled).
       *
       * If the object is already queued and/or running, this function
       * will not queue again. Instead, this function will make the call
       * block until the resolution is finished one way or another.
       *
       * Also note that you may re-start a canceled or failed request.
       *
       * @return true upon successful resolution or false if we failed
       */
      bool start();

      /**
       * Starts the name resolution asynchronously.
       *
       * This function will queue this object for resolution
       * and will return immediately. The status upon exit will either be
       * Queued or Failed.
       *
       * This function does nothing if the object is already queued. But if
       * it had already succeeded or failed, this function will re-start it.
       *
       * @return true if this request was successfully queued for asynchronous
       *		resolution
       */
      bool startAsync();

      /**
       * Cancels a running request
       *
       * This function will cancel a running request. If the request is not
       * currently running or queued, this function does nothing.
       *
       * @param emitSignal	whether to emit the @ref finished signal or not
       */
      void cancel(bool emitSignal = true);

      /**
       * Retrieves the results of this resolution
       *
       * Use this function to retrieve the results of the resolution. If no
       * data was resolved (yet) or if we failed, this function will return
       * an empty object.
       *
       * @return the resolved data
       * @see status for information on finding out if the resolution was successful
       */
      ResolverResults results() const;

    signals:
      // signals

      /**
       * This signal is emitted whenever the resolution is finished, one
       * way or another (success or failure). The @p results parameter
       * will contain the resolved data.
       *
       * Note: if you are doing multiple resolutions, you can use the 
       * QObject::sender() function to distinguish one Resolver object from
       * another.
       *
       * @param results		the resolved data; might be empty if the resolution
       *			failed
       * @see results for information on what the results are
       */
      void finished(KDE::Network::ResolverResults results);

    public:
      // Static functions

      /**
       * Returns the string representation of this error code.
       *
       * @param error	the error code. See @ref ErrorCodes.
       * @param syserror	the system error code associated.
       * @return		the string representation. This is already
       *			i18n'ed.
       */
      static QString strError(int errorcode, int syserror = 0);

      /**
       * Resolve the nodename and service name synchronously
       *
       * This static function is provided as convenience for simplifying
       * name resolution. It resolves the given host and service names synchronously
       * and returns the results it found. It is equivalent to the following code:
       *
       * <code>
       *   Resolver kres(host, service);
       *   kres.setFlags(flags);
       *   kres.setFamily(families)
       *   kres.start();
       *   return kres.results();
       * </code>
       *
       * @param host		the nodename to resolve
       * @param service		the service to resolve
       * @param flags		flags to be used
       * @param families	the families to be searched
       * @return a ResolverResults object containing the results
       * @see ResolverResults for information on how to obtain the error code
       */
      static ResolverResults resolve(const QString& host, const QString& service,
				     int flags = 0, int families = Resolver::AnyFamily);

      /**
       * Start an asynchronous name resolution
       *
       * This function is provided as a convenience to simplify the resolution
       * process. It creates an internal Resolver object, connects the
       * @ref finished signal to the given slot and starts the resolution
       * asynchronously. It is more or less equivalent to the following code:
       *
       * <code>
       *   Resolver* kres = new Resolver(host, service);
       *   QObject::connect(kres, SIGNAL(finished(ResolverResults)),
       *			  userObj, userSlot);
       *   kres->setFlags(flags);
       *   kres->setFamily(families);
       *   return kres->startAsync();
       * </code>
       *
       * You should use it like this in your code:
       * <code>
       *   Resolver::resolveAsync(myObj, SLOT(mySlot(KResolveResults)),
       *				 host, service);
       * </code>
       *
       * @param userObj		the object whose slot @p userSlot we will connect
       * @param userSlot	the slot to which we'll connect
       * @param host		the nodename to resolve
       * @param service		the service to resolve
       * @param flags		flags to be used
       * @param families	families to be searcheed
       * @return true if the queueing was successful, false if not
       * @see ResolverResutls for information on how to obtain the error code
       */
      static bool resolveAsync(QObject* userObj, const char *userSlot,
			       const QString& host, const QString& service,
			       int flags = 0, int families = Resolver::AnyFamily);

      /**
       * Resolves a socket address to its textual representation
       *
       * FIXME!! How can we do this in a non-blocking manner!?
       *
       * This function is used to resolve a socket address from its
       * binary representation to a textual form, even if numeric only.
       *
       * @param addr	the socket address to be resolved
       * @param node	the QString where we will store the resolved node
       * @param serv	the QString where we will store the resolved service
       * @param flags	flags to be used for this resolution.
       * @return true if the resolution succeeded, false if not
       * @see ReverseFlags for the possible values for @p flags
       */
      static bool reverseResolve(const KSocketAddress* addr, QString& node, 
				 QString& serv, int flags = 0);

      /**
       * Resolves a socket address to its textual representation
       *
       * FIXME!! How can we do this in a non-blocking manner!?
       *
       * This function behaves just like the above one, except it takes
       * a sockaddr structure and its size as parameters.
       *
       * @param sa	the sockaddr structure containing the address to be resolved
       * @param salen	the length of the sockaddr structure
       * @param node	the QString where we will store the resolved node
       * @param serv	the QString where we will store the resolved service
       * @param flags	flags to be used for this resolution.
       * @return true if the resolution succeeded, false if not
       * @see ReverseFlags for the possible values for @p flags
       */
      static bool reverseResolve(const struct sockaddr* sa, ksocklen_t salen, 
				 QString& node, QString& serv, int flags = 0);

      /**
       * Returns the domain name in an ASCII Compatible Encoding form, suitable
       * for DNS lookups. This is the base for International Domain Name support
       * over the Internet.
       *
       * Note this function may fail, in which case it'll return a null 
       * QCString. Reasons for failure include use of unknown code
       * points (Unicode characters).
       *
       * Note that the encoding is illegible and, thus, should not be presented
       * to the user, except if requested.
       *
       * @param domain	the domain name to be encoded
       * @return the ACE-encoded suitable for DNS queries if successful, a null
       * QCString if failure.
       */
      static QCString domainToAscii(const QString& unicodeDomain);

      /**
       * Does the inverse of @ref domainToAscii and return an Unicode domain
       * name from the given ACE-encoded domain.
       *
       * This function may fail if the given domain cannot be successfully
       * converted back to Unicode. Reasons for failure include a malformed
       * domain name or good ones whose reencoding back to ACE don't match
       * the form given here (e.g., ACE-encoding of an already
       * ASCII-compatible domain).
       *
       * It is, however, guaranteed that domains returned
       * by @ref domainToAscii will work. And the following is always true,
       * if the encoding is successful:
       * <code>
       *    domainToUnicode(domainToAscii(domainname)) == domainname
       * </code>
       *
       * @param domain	the ACE-encoded domain name to be decoded
       * @return the Unicode representation of the given domain name
       * if successful, the original string if not
       * @note ACE = ASCII-Compatible Encoding, i.e., 7-bit
       */
      static QString domainToUnicode(const QCString& asciiDomain);

      /**
       * The same as above, but taking a QString argument.
       *
       * @param domain	the ACE-encoded domain name to be decoded
       * @return the Unicode representation of the given domain name
       * if successful, QString::null if not.
       */
      static QString domainToUnicode(const QString& asciiDomain);

      /**
       * Normalise a domain name.
       *
       * In order to prevent simple mistakes in International Domain
       * Names (IDN), it has been decided that certain code points
       * (characters in Unicode) would be instead converted to others.
       * This includes turning them all to lower case, as well certain
       * other specific operations, as specified in the documents.
       *
       * For instance, the German 'ß' will be changed into 'ss', while
       * the micro symbol 'µ' will be changed to the Greek mu 'μ'.
       *
       * Two equivalent domains have the same normalised form. And the
       * normalised form of a normalised domain is itself (i.e., if 
       * d is normalised, the following is true: d == normalizeDomain(d) )
       *
       * This operation is equivalent to encoding and the decoding a Unicode
       * hostname.
       *
       * @param domain		a domain to be normalised
       * @return the normalised domain, or QString::null if the domain is
       * invalid.
       */
      static QString normalizeDomain(const QString& domain);

      /**
       * Resolves a protocol number to its names
       *
       * Note: the returned QStrList operates on deep-copies.
       *
       * @param protonum	the protocol number to be looked for
       * @return all the protocol names in a list. The first is the "proper"
       *		name.
       */
      static QStrList protocolName(int protonum);

      /**
       * Finds all aliases for a given protocol name
       *
       * @param protoname	the protocol name to be looked for
       * @return all the protocol names in a list. The first is the "proper"
       *		name.
       */
      static QStrList protocolName(const char *protoname);

      /**
       * Resolves a protocol name to its number
       *
       * @param protoname	the protocol name to be looked for
       * @return the protocol number or -1 if we couldn't locate it
       */
      static int protocolNumber(const char *protoname);

      /**
       * Resolves a service name to its port number
       *
       * @param servname		the service name to be looked for
       * @param protoname		the protocol it is associated with
       * @return the port number in host byte-order or -1 in case of error
       */
      static int servicePort(const char *servname, const char *protoname);

      /**
       * Finds all the aliases for a given service name
       *
       * Note: the returned QStrList operates on deep-copies.
       *
       * @param servname		the service alias to be looked for
       * @param protoname		the protocol it is associated with
       * @return all the service names in a list. The first is the "proper"
       *		name.
       */
      static QStrList serviceName(const char *servname, const char *protoname);

      /**
       * Resolves a port number to its names
       *
       * Note: the returned QStrList operates on deep copies.
       *
       * @param portnum		the port number, in host byte-order
       * @param protoname		the protocol it is associated with
       * @return all the service names in a list. The first is the "proper"
       *		name.
       */
      static QStrList serviceName(int port, const char *protoname);
 
    protected:

      /**
       * Sets the error codes
       */
      void setError(int errorcode, int systemerror = 0);

      /// @internal
      virtual void virtual_hook( int id, void* data );
    private:
      ResolverPrivate* d;
      friend class ResolverResults;
    };

  } // namespace KDE::Network
} // namespace KDE

#endif
