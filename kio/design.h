/**
 * @libdoc A network-enabled file management system in a library.
 *
 * This library implements almost all the file management functions you
 * will ever need. In fact, the KDE file manager, Konqueror also uses
 * this to provide its network-enabled file management.
 *
 * The easiest way to use this library from a KDE application is via the
 * @ref KIONetAccess class (for easy synchronous access) or via the
 * @ref KIOJob class (for more complex asynchronous jobs).
 *
 * This library also implements the System Configuration Cache (KSycoca).
 *
 * @ref KMimeType : 
 *   The notion of a file type, already existing in KDE-1.x
 *
 * @ref KService :
 *   To a mimetype are bound one or more applications, now called services.
 *   Services can be applications, but also libraries, dynamically opened.
 * 
 * @ref KServiceType :
 *   A service type allows the same mechanism to be extended to components.
 *   For instance : the question "what are the koffice plugins" is solved by
 *   a service type called KOfficePlugin, and by every plugin providing
 *   a .desktop file stating that it is a service that implements the servicetype
 *   KOfficePlugin.
 * 
 * @ref KServiceTypeProfile :
 *   Stores the user's preferences for services bound to mimetypes/servicetypes.
 * 
 * @ref KTrader :
 *   Use it to query for which services implement a given mimetype/servicetype.
 *   Has its own language, in order to allow complex queries.
 */
