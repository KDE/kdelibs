/**
 * @libdoc A network-enabled file management system in a library.
 *
 * This library implements almost all the file management functions you
 * will ever need. In fact, the KDE file manager, Konqueror also uses
 * this to provide its network-enabled file management.
 *
 * The easiest way to use this library from a KDE application is via the
 * @ref KIO::NetAccess class (for easy synchronous access) or via the
 * @ref KIO::Job class (for more complex asynchronous jobs).
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
 *

 * \section KDE Filedialog widget and associated classes.
 *
 * This library also provides the KDE file selector widget,
 * its building blocks and some other widgets, making use of the file dialog.
 *
 * The file dialog provides different views; there is a vertically scrolling
 * view based on @ref KListView, showing things like filename, file size,
 * permissions etc. in separate columns. And there is a horizontally scrolling
 * view based on @ref KIconView. Additionally, there are some compound views,
 * like a view using the icon-view for files on the right side and another
 * view for directories on the left. A view, that shows a preview for the
 * currently selected file (using @ref KIO::PreviewJob to generate previews)
 * and any other view to show the files is also available.
 *
 * All those views share a common baseclass, named @ref KFileView, which
 * defines the interface for inserting files into a view, removing them,
 * selecting etc.
 *
 * The one class encapsulating all those views and adding browsing capabilities
 * to them is @ref KDirOperator. It allows the user to switch between different
 * views.
 *
 * @ref KFileTreeView is a KListView based widget that displays files and/or
 * directories as a tree. It does not implement the KFileView interface,
 * however, so it can't be used with KDirOperator.
 *
 * Besides the filebrowsing widgets, there is the @ref KPropertiesDialog class,
 * implementing a dialog showing the name, permissions, icons, meta
 * information and all kinds of properties of a file, as well as providing a
 * means to modify them.
 *
 * The KPropertiesDialog is extensible with plugin-pages via the
 * @ref KPropsDlgPlugin class.
 *
 * The @ref KIconDialog class shows a list of icons installed on the system (as
 * accessible via @ref KIconLoader) and allows the user to select one.
 *
 * @ref KOpenWithDlg implements a dialog to choose an application from, that is
 * to be run, e.g. to let the user choose an application to open a file/url
 * with.
 *
 *
 * @ref KFileDialog :
 *   The class providing the file selector dialog. It combines a KDirOperator,
 *   KURLBar and several other widgets.
 *
 * @ref KDirOperator :
 *   The class encapsulating different KFileViews, offering file browsing and
 *   file selection. Asynchronous, network transparent reading of directories
 *   is performed via the @ref KIO library.
 *
 * @ref KURLRequester :
 *   A widget to be used for asking for a filename/path/URL. It consists of a
 *   @ref KLineEdit (or @ref KComboBox) and a button. Activating the button
 *   will open a @ref KFileDialog. Typing in the lineedit is aided with
 *   filename completion.
 *
 * @ref KURLRequesterDlg  :
 *   A dialog to ask for a filename/path/URL, using KURLRequester.
 *
 * @ref KFileView  :
 *   The base class for all views to be used with KDirOperator.
 *
 * @ref KFileIconView :
 *   The KFileView based on KIconView.
 *
 * @ref KFileDetailView :
 *   The KFileView based on KListView.
 *
 * @ref KFilePreview :
 *   The KFileView, combining a widget showing preview for a selected file
 *   and another KFileView for browsing.
 *
 * @ref KURLBar :
 *   A widget offering a number of clickable entries which represent a URL,
 *   aligned horizontally or vertically. The entries are customizable by the
 *   user both on a per application basis or for all applications (URLs, their
 *   icon and the description can be added, removed or edited by the user).
 *   This is the widget used as "sidebar" in the @ref KFileDialog.
 *
 * @ref KFileMetaInfoWidget :
 *   A widget that allows viewing and editing of meta data of a file, utilizing
 *   @ref KFileMetaInfo.
 *
 */
