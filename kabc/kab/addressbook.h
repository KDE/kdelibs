/* -*- C++ -*-
   This file declares the basic personal information management class
   used in the KDE addressbook.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $
*/

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

class KeyValueMap;
class QConfigDB; 
class Section;
class StringKabKeyMap; /* The type of the mirror map. */
class QStringList;
/* Used to implement field lookup accoording to
   keys. */
class KeyNameMap; 

#include <list>
#include <q3frame.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qsize.h>
#include <qvariant.h>
#include <qmap.h>
#include <q3cstring.h>

/**
 * The class KabKey is used to select entries out of the database file.
 * In future, keys might become more complex.
 */
class KabKey
{
public:
  /**
   * The comparison operator
   */
  bool operator==(const KabKey&) const;
  /**
   * Get the key as a QCString
   */
  Q3CString getKey() const;
  /**
   * Set this key
   */
  void setKey(const Q3CString&);
protected:
  /**
   * The key of the in this database
   */
  Q3CString key;

  /** Class for private data. */
  class KabKeyPrivate;
  /** Private data pointer. @internal */
  KabKeyPrivate *d;
};

class CategoriesMap : public QMap<int, QString>
{
};

// -----------------------------------------------------------------------------
// this will be incremented when kab's file format changes significantly:
#if defined KAB_FILE_FORMAT
#undef KAB_FILE_FORMAT
#endif
/*
  0:  all formats before the email list was implemented
  1:  format enhanced for unlimited number of email addresses
  2:  format enhanced by more address fields
  10: format of kab 2
  11: added categories
*/
#define KAB_FILE_FORMAT 11

// -----------------------------------------------------------------------------
// this defines will contain the program version used for different purposes:
#ifdef KAB_VERSION
#undef KAB_VERSION
#endif
#ifdef KAB_MINOR
#undef KAB_MINOR
#endif
#ifdef KAB_PATCH
#undef KAB_PATCH
#endif
#ifdef KAB_STATE
#undef KAB_STATE
#endif
#define KAB_VERSION 2
#define KAB_MINOR   2
#define KAB_PATCH   0
#define KAB_STATE   "final"

// -----------------------------------------------------------------------------
/** The class AddressBook implements the base class for the KDE addressbook. 
 *  \par Overview
 *  It
 *  is used by the KabAPI to make the interface to kab files available to 
 *  application programmers. <BR> 
 *  Unlike in the first kab version, the configuration file and the data file are 
 *  different objects of QConfigDB.  This way,  the data file is no more limited
 *  to the one in the users KDE directory, multiple files may be used.  Different
 *  instances of the program may use different data files.  Read-only addressbook
 *  files are possible. <BR>
 *  Only one configuration file per user is used, it is <BR>
 *  <TT>   ~/.kde/share/apps/kab/kab.config </TT> <BR>
 *  A standard user file will automatically be created as <BR> 
 *  <TT>   ~/.kde/share/apps/kab/addressbook.kab </TT> <BR>
 *  File changes are watched by the program, so every instance will automatically
 *  update its database on a change of the opened file. 
 *
 *  \par The KDE addressbook database system
 *  kab manages entries in address databases based on a key system where the 
 *  program assigns keys to added entries. These keys are not reused in one file, 
 *  so API users can rely on a key to be unique and identifying until the entry 
 *  is deleted by the user (this is a change to kab 1 that reused freed entry 
 *  keys). Of course, in different files a key might be used twice. <BR>
 *  The keys are objects of the type KabKey and define the section in the 
 *  addressbook database where the entry is stored (see QConfigDB
 *  reference). Keys invalidate on file changes, so keep track of the
 *  signal ::changed. <BR>
 *  kab watches file changes. If the opened file changes on disk, it is
 *  automatically reloaded and ::changed() is emitted. 
 *
 *  \par The users standard personal information database
 *  kab assumes that it is able to read and write the users standard database.
 *  This way, the kab application itselfes and applications using the KabAPI can
 *  rely on the possibility to add entries to this database (from a browser, for
 *  example). Usually, this file is opened automatically by the constructor. 
 *  If - for what reason ever - the file cannot be either created or read, kab 
 *  will still start up, but no database operation will work until the user 
 *  opened a file. In this case, the method ::getState will return 
 *  ::PermDenied. In general it is a good idea to check the return value of the
 *  ::getState method before using KabAPI operations.
 *
 *  \par The mirror map
 *  The entries are stored in the QConfigDB object ::data which represents the
 *  currently opened file. In every file there is a section with the name 
 *  <TT> entries </TT> that contains a subsection for every entry. The name of
 *  the subsection is the key of the entry. <BR>
 *  When retrieving the sections, they are ordered alphabetically by their keys.
 *  This is not what users expect, since the keys show the insertion order of 
 *  the entries, not more and not less. Additionally the displaying order should
 *  be configurable. <BR>
 *  That is why kab uses a STL map to map its entry keys to user
 *  (at least programmer...) defined descriptors. Usually, the descriptors are 
 *  created as a combination of the entry data, and then displayed in aphabetical 
 *  order in the selector combobox. This map is called the mirror map throughout 
 *  the documentation. It is created or updated every time the database changes.
 *  Thus the way to find a special entry is: <OL>
 *  <LI> the user selects an item in the selector combo box, returning its 
 *       index, </LI>
 *  <LI> the index is used to find the key of the entry in the mirror map, </LI>
 *  <LI> and finally the entry is retrieved by its key from the database. </LI>
 *  </OL>
 *  To modify the sorting order, the way to create the entry descriptors in the 
 *  mirror map nedds to be changed. 
 *
 *  \par The view
 *  If you display an AddressBook object (that is a derived QFrame), 
 *  it may show an entry 
 *  of the database that you might select. The entry you hand over to the method
 *  ::displayEntry does not need to be contained in the currently loaded file.
 *  This way you may integrate views of 
 *  the users addressbook database in your own application as a simple widget 
 *  type. To allow the user to
 *  navigate through the database, you might want to show kab's own toolbar in 
 *  your mainwindow (or whereever). (The toolbar is not implemented by now). <BR>
 *  Some parts of the AddressBook widget are \e interactive, that means they are
 *  displayed as transparent KURLLabels that react when the user clicks on it.
 *  These interactive parts have to be enabled by calling #setInteractiveMode.
 */
class AddressBook : public Q3Frame
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /**
   * The return values of some AddressBook member functions are #ErrorCode
   * values.
   */
  enum ErrorCode {
    NoError, /** No error, the operation did not fail. */
    PermDenied, /**< Access permissions for the operation are not available. */
    Locked, /**< An writing operation on a locked file was requested. */
    Rejected, /**< The requested operation has been rejected by the user. */
    NoSuchEntry, /**< An entry has been referenced using a unknown key. */
    NoEntry, /**< You tried to retrieve an entry but there is none. */
    NoFile, /**< No file has been loaded by now. */
    NoSuchFile, /**< A filename could not be found on the filesystem. */
    InternError, /**< A error in kab's internal logic occurred. */
    OutOfRange, /**< An index value was out of the allowed range. */
    NoSuchField, /**< You queried a field that does not exist. */ 
    NotImplemented /**< The requested operation is not implemented. */
  };
  /**
   * Some predefined telephone types. More are possible, but these are
   * provided and thus, for example, translated.
   */
  enum Telephone {
    NoTelephone,
    Fixed,
    Mobile,
    Fax,
    Modem,
    User1,
    User2,
    User3,
    NoOfTelephoneTypes
  };
  /** Each entry in a loaded database has its own ::Entry object.
   *
   *  \par The structure of the address database
   *  As you might have read, kab uses the QConfigDB class to manage its
   *  data files. This class is intended to handle hierarchical structures.
   *  Thus, kab is able to create human readable but still deep and complex
   *  data files. This paragraph describes the overall structure of the
   *  files, the next two deal with special parts of it. <BR>
   *  First of all, kab II data files (that usually end with \c .kab, while in
   *  kab 1 the fixed file name was \c addressbook.database) have two main
   *  sections (see the documentation of the QConfigDB and Section classes),
   *  one is called \c config, it contains different file specific
   *  configuration settings like the last displayed entry, and one section
   *  called \c entries that in turn contains a subsection for each entry in
   *  the database file. The keys of this subsections are the literal strings
   *  that are used in the KabKey class in the member KabKey::key. Each entry
   *  subsection has some key-value-pairs described below and another
   *  subsection "addresses" with one or more addresses in it. See the
   *  following example for a kab II data file (without any key-value-pairs):
   *  <BR> <PRE>
   *  [config]
   *  [END]
   *  [entries]
   *     [1] (the first entry with literal key "1")
   *       [addresses]
   *         [1] (the first address, addresses are enumerated)
   *         [END]
   *         [2] (the second address)
   *         [END]
   *         ... (more addresses may follow)
   *       [END]
   *     [END]
   *     [2] (the second entry)
   *       [addresses]
   *         [1]
   *         [END]
   *       [END]
   *     [END]
   *     ... (more entries may follow)
   *  [END] </PRE> <BR>
   *
   *  \par The fields an entry contains
   *  An entry contains all settings that are expected to be unique for all
   *  addresses directly as key-value-pairs. Everything that is part of a
   *  specific address of this person is part of an object of the member list
   *  \c addresses referenced in the next paragraph.
   *  The keys defined directly in the entry sections are:
   *    -  "title" The title of that person.
   *    -  "rank" A possible military rank of that person.
   *    -  "fn" The formatted name. If it is not empty, it replaces the
   *       standard combination of the other name fields in the address
   *       display.
   *    -  "nameprefix" A possible name prefix.
   *    -  "firstname" The first name.
   *    -  "middlename" The middle name.
   *    -  "lastname" The last name.
   *    -  "birthday" The birthday (a QDate).
   *    -  "comment" A free form comment.
   *    -  "talk" The talk addresses (a string list).
   *    -  "emails" The email addresses (a string list).
   *    -  "keywords" A list of free-form keywords.
   *    -  "telephone" A list of telephone numbers in a special format.
   *    -  "URLs" A list of internet addresses.
   *    -  "user_1" The first user-declared data field.
   *    -  "user_2" The second user-declared data field.
   *    -  "user_3" The third user-declared data field.
   *    -  "user_4" The fourth user-declared data field.
   * 
   *  See the next section for a description of the addresses subsections.
   * 
   *  \par The fields of the addresses subsections
   *  The section for each entry contains a subsection \c addresses with
   *  in turn a subsection for each address. The addresses are enumerated
   *  in the order they are inserted, their keys are the numbers of
   *  inserting converted to a string.
   *  The keys defined in an address subsection are:
   *    -  "headline" A headline shown for the address.
   *    -  "position" The position of the person.
   *    -  "org" The organization.
   *    -  "orgunit" The organizational unit.
   *    -  "orgsubunit" The organizational subunit.
   *    -  "role" The role of the person.
   *    -  "deliverylabel" A label for delivering to this address.
   *    -  "address" The street, house no., flat etc line.
   *    -  "zip" A zip or postal code.
   *    -  "town" The town the person lives in in this address.
   *    -  "country" The country for federal states.
   *    -  "state" The state for federal states.
   *
   *  \par The local configuration section
   *  For each kab II database file there are some settings that apply
   *  only to the file itselfes, not to all kab databases the user works
   *  with. These settings are called the local configuration. The settings
   *  are stored in the \c config section of the local file. The following
   *  keys are declared in this section:
   *    -  "user_1" The \e name of the first user-declared field.
   *    -  "user_2" The \e name of the second user-declared field.
   *    -  "user_3" The \e name of the third user-declared field.
   *    -  "user_4" The \e name of the fourth user-declared field.
   * 
   *  More fields will surely follow.
   **/
  class Entry {
  public:
    // types:
    /** Since an entry may have different addresses, we need a type for them.
     *  Multiple addresses are used to distinguish between addresses at home
     * and work, for example. */
    class Address {
    public:
      /** A constructor. */
      Address();
      // ----- This aggregates are used to access the fields by
      // keywords. We use char* here to be able to initialize the keys
      // in code as statics without initializing Qt etc. :
      /** An aggregat containing the keys of all declared fields:
       */
      static const char* Fields[];
      /** The number of elements in Fields.
       */
      static const int NoOfFields;
      /** Query the literal, translated name of the field given by its
	  key. 
	  @return false if key is not defined */
      static bool nameOfField(const char* key, QString& value);
      /** Get a field by its field name. Field names are defined in
	  @see Fields. Since there are different file types a field
	  may be represented with, a QVariant is returned. */
      ErrorCode get(const char* key, QVariant&);
      // ----- the following members represent the fields:
      /** The headline for this address. */
      QString headline; 
      /** The position of the person at this address. */
      QString position; 
      /** The organization of the person at this address. */
      QString org; 
      /** The org unit of the person at this address. */
      QString orgUnit;  
      /** The org subunit of the person at this address. */
      QString orgSubUnit; 
      /** The description for delivering. */
      QString deliveryLabel; 
      /** Street, with house number. */
      QString address; 
      /** Zip or postal code. */
      QString zip;
      /** The town. */
      QString town; 
      /** The country for federal states. */
      QString country;
      /** The state for federal states. */ 
      QString state; 
    protected:
      /** Map of field names */
      static KeyNameMap *fields;
    };
    /** Contains one or more Address objects. */
    std::list<AddressBook::Entry::Address> addresses; 
    // ----- This aggregates are used to access the fields by
    // keywords. We use char* here to be able to initialize the keys
    // in code as statics without initializing Qt etc. :
    /** An aggregat containing the keys of all declared fields:
     */
    static const char* Fields[];
    /** The number of elements in Fields.
     */
    static const int NoOfFields;
    // methods:
    /** Use this method to retrieve the address at the given \a index.
     *  The method is provided for convenience. The address data is
     *  returned in \a address. */
    AddressBook::ErrorCode getAddress(int index, Address& address) const;
    /** Returns the number of addresses of this entry. */
    int noOfAddresses() const;
    /** Query the literal, translated name of the field given by its
	key. 
	@return false if key is not defined */
    static bool nameOfField(const char* key, QString& value);
    /** Get a field by its field name. Field names are defined in
	@see Fields. Since there are different file types a field
	may be represented with, a QVariant is returned. */
    ErrorCode get(const char* key, QVariant&);
    // members:
    // this parts are assumed to be unique for every entry:
    QString title; /**< The title of the person. */
    QString rank; /**< The rank of the person. */
    QString fn; /**< The formatted name of the person. */
    QString nameprefix; /**< A possibly name prefix for that person. */
    QString firstname; /**< The first name of the person. */
    QString middlename; /**< The middle name of the person. */
    QString lastname; /**< The last name of the person. */
    QDate birthday; /**< The birthday of this person. */
    QString comment; /**< The comment. */
    QStringList talk;  /**< The talk addresses. */
    QStringList emails; /**< The email addresses. */
    QStringList keywords; /**< The user defined keywords for searching. */
    /**
     * Telephone numbers and types. This list contains combinations of telephone
     * numbers and the types of the phones, in this order. See enum
     * Telephone above.
     */
    QStringList telephone; 
    QStringList URLs; /**< The home or related web pages of this person. */
    QString user1; /**< The first user-declared field. */
    QString user2; /**< The second user-declared field. */
    QString user3; /**< The third user-declared field. */
    QString user4; /**< The fourth user-declared field. */    
    QStringList custom;     /**< Custom fields (application specific). */
    QStringList categories; /**< The categories this entry is assigned to. */
  protected:
    /** Map of field names */
    static KeyNameMap *fields;
  };
  /**
   * The constructor. If \e load is true, the user standard file will
   * automatically be loaded into the object.
   */
  AddressBook(QWidget* parent=0, const char* name=0, bool load=true);
  ~AddressBook(); /**< The destructor. */
  /**
   * Get the internal state of the object. 
   * If no problem occurred, it returns ::NoError. 
   * If the standard or the latest opened file could not be loaded,
   * it returns ::PermDenied
   */
  ErrorCode getState();
  /**
   * Load the file with the given path. An empty file name reloads the 
   * currently opened file.
   */
  ErrorCode load(const QString& filename=QString::null);
  /**
   * Save the file to the given path and file name.  An empty file name saves 
   * to the file where the database has been read from.
   * If force is true, the method will switch to r/w mode for saving and
   * back.
   */
  ErrorCode save(const QString& filename=QString::null, bool force=false);
  /**
   * Close this file. 
   * ::closeFile assures sure that the ::data object is reset no matter of the 
   * state of the assigned file. 
   * If \a save is true, it will not close the file if it could not be 
   * saved.
   */
  ErrorCode closeFile(bool saveit=true);
  /**
   * Retrieve an entry from the database by its key.
   */
  ErrorCode getEntry(const KabKey& key, Entry&);
  /**
   * Retrieve the Section of the entry directly, returning a section object.
   */
  ErrorCode getEntry(const KabKey& key, Section*&);
  /**
   * Get all entries in displaying order. This method might be slow (O(n)).
   */
  ErrorCode getEntries(std::list<Entry>&);
  /**
   * Add an ::Entry, \a return the new key for further operations.
   * If update is false, the mirror map will not be affected, if it is true,
   * the mirror map gets updated, too.
   */
  ErrorCode add(const Entry&, KabKey& key, bool update=true);
  /**
   * Set the entry with the given key to the new contents. Be aware of
   * #PermDenied for read-only databases or file sharing conflicts. You cannot 
   * change entries in a database for which you do not have write access.
   */
  ErrorCode change(const KabKey& key, const Entry&);
  /**
   * Remove the  entry with the given key. Returns #NoSuchEntry if there is no 
   * entry with this key, #PermDenied for read only databases.
   */
  ErrorCode remove(const KabKey& key);
  /**
   * Returns the number of entries in the loaded database.
   */
  unsigned int noOfEntries();
  /**
   * This method returns the literal name for the entry, 
    * containing either the formatted name (if given) or a 
    * combination of the first, additional and last name. 
    * The name is returned in \a text.
    * If \a reverse is false, the text looks like
    *    firstname (add. name) last name,
    * if it is true, 
    +    last name, first name (add. name).
    * If \a initials is true, the text contains initials only:
    *    f. a. name [with reverse==false] or
    *    name, f. a. [with reverse==true].
    * If there is no entry with this key, the method returns ::NoSuchEntry.
    */
  ErrorCode literalName(const KabKey& key, QString& text,
			bool reverse=false, bool initials=false);
  /**
   * This is an overloaded method that differs only in the arguments it takes.
   */
  ErrorCode literalName(const Entry& entry, QString& text,
			bool reverse=false, bool initials=false);
  /**
   * Get the key of the item in the selector with the given index.
   */
  ErrorCode getKey(int index, KabKey&);
  /**
   * Get the index of this key in the selector. This is the reverse
   * functionality to getKey().
   */
  ErrorCode getIndex(const KabKey&, int&);
  /**
   * Fill the string list with name lines. If your application shows a combobox 
   * containing an overview over the currently loaded KabAPI database, then 
   * call this method when receiving the signal ::changed and display the list
   * in the combo.
   */
  ErrorCode getListOfNames(QStringList*, bool reverse=true, bool initials=true);
  /**
   * Hand over the configuration database. Careful!
   */
  QConfigDB* getConfig(); 
  /**
   * This method returns the QConfigDB section where the configuration of the
   * currently opened file is stored. It might be used to retrieve or to modify
   * these settings. The file-specific settings are saved along with
   * the open file.
   * Do not confuse the configuration section of the opened file with 
   * the configuration of the program. Each file might have its own
   * local configuration for some settings where it makes sense. 
   * @ return Null if no file has been opened. 
   */
  Section *configurationSection();
  /**
   * This method opens a dialog for configuring the file-specific settings
   * for the loaded file. The database is automatically saved if the user
   * accepts the changes.
   */
  // ErrorCode configureFile();
  /**
   * Creates a new database with the given file name. If the filename is 
   * empty, it creates the users standard data file. The method does not load
   * the new database.
   */
  ErrorCode createNew(const QString& filename=QString::null);
  /**
   * Creates the local configuration file. The filename is fixed to
   * \c kab.config, it will be created in the local kab directory
   * (\c $HOME/.kde/share/apps/kab). Adapt the global configuration template
   * file (\c $KDEDIR/share/apps/kab/template.config) for unusual site-specific
   * settings.
   * The method does not load the new config file.
   */
  ErrorCode createConfigFile();
  ErrorCode loadConfigFile(); /**< Load the local configuration file. */
  // ErrorCode configureKab(); /**< Open the configuration dialog for the KabAPI. */
  // QSize sizeHint();  /**< The preferred (minimal) size of the view. */ // ni
  /**
   * This method parses a vCard and creates an Entry object from it.
   */
  ErrorCode makeEntryFromVCard(const QString& card, Entry&);
  /**
   * This method creates a vCard string from an entry.
   */
  ErrorCode makeVCardFromEntry(const Entry& entry, const QString& card);
  /**
   * Returns the complete path to the user standard file. An empty path
   * indicates an error, but this should not happen. It is NOT ensured
   * that the file exists.
   */
  QString getStandardFileName();
  /**
   * Call this to get a telephone type translated to the locale.
   */
  static QString phoneType(AddressBook::Telephone);
  /**
   * Query the entry categories defined for this address
   * book. Categories may differ between addressbooks.
   */
  ErrorCode categories(CategoriesMap& categories);
  /**
   * Modify the categories for this addressbook. The map given will replace the 
   * previoulsy stored one.
   */
  ErrorCode setCategories(const CategoriesMap& categories);
  /**
   * Query the real name of a category by its index.
   */
  ErrorCode category(int index, QString&);
  /**
   * Query the category section. This is the "raw" storage of the defined 
   * categories. It is always defined (or will be created if you have an old 
   *  file that does not have categories).
   *  @see Section
   */
  Section* categoriesSection();
  // ----------------------------------------------------------------------------

#ifdef KDE_NO_COMPAT
private:
#endif
    /** Hack for once-broken camelcaps. @deprecated */
    QString getStandardFilename() { return getStandardFileName(); };

protected:
  QConfigDB *config; /**< The configuration database. */
  QConfigDB *data; /**< The currently open data files. */
  StringKabKeyMap *entries; /**< The mirror map. */
  ErrorCode state; /**< The internal state of the object. */
  /**
   * Get the next available entry key for this file. For internal use only.
   */
  KabKey nextAvailEntryKey();
  /**
   * Returns true if both pathes point to the same file.
   *  The method resolves relative file names to find this out.
   */
  bool isSameFile(const QString& a, const QString& b);
  /**
   * Parse the section and copy its contents into \a entry.
   * The method expects a subsection called \e addresses that contains a
   * number of subsections each containing data for one Entry::Address object.
   * All other fields are copied directly into the members of \a entry.
   */
  ErrorCode makeEntryFromSection(Section*, Entry&); // nicht beendet
  /**
   * For internal use only. This parses one address subsection and puts its
   * contents in the Address object.
   */
  ErrorCode makeAddressFromMap(KeyValueMap*, Entry::Address&);
  /**
   * Create a section from the entries settings.
   */
  ErrorCode makeSectionFromEntry(const Entry&, Section&); // nicht beendet
  /**
   * Update the mirror map after changes of the database.
   */
  ErrorCode updateMirrorMap();
  /**
   * Get the entry section of the file. Maybe a NULL pointer if no file is
   * opened.
   */
  Section* entrySection();
  /**
   * Lock the file for changing.
   * Since all database files are opened read-only, they must be locked before
   * the files contents are changed. After changing the file must be saved and
   * unlocked. Returns ::PermDenied if the file could not be locked, ::NoError
   * if it was not locked and is now, and ::Locked if the file is already
   * locked.
   * @see unlock
   * @see QConfigDB::setFileName
   */
  ErrorCode lock();
  /**
   * Unlock the file after changes. Returns ::NoError if the file was locked
   * and could be unlocked, ::PermDenied if the file was not locked and
   * possibly ::InternError if anything fails.
   * @see ::lock
   * @see QConfigDB::setFileName
   */ 
  ErrorCode unlock();
  /**
   * Set the background image. Kab will store a deep copy of the image.
   * If the image is a null image nothing will be displayed.
   */
  // void setBackground(const QImage&);
  /**
   * Enable or disable the background image.
   */
  // void setBackgroundEnabled(bool state);
  /**
   * Retrieve wether the background image is enabled or not.
   */
  // bool getBackgroundEnabled();
  /**
   * Set if the URL labels are interactive.
   */
  // void setInteractiveMode(bool state);
  /**
   * Get if the URL labels are interactive.
   */
  // bool getInteractiveMode();
protected slots:
  /**
   * Called when ::data has been cleared or reloaded.
   */
  void reloaded(QConfigDB*);
  /**
   * Called when the \e file assigned to ::data has changed on disk.
   */
  void dataFileChanged();
  /**
   * Called when the \e file assigned to ::config has changed on disk.
   */
  void configFileChanged();
  // ----------------------------------------------------------------------------
public slots:
  /**
   * This slot is called when an external object changed the database through
   * the kabapi.
   */
  void externalChange(); 
  // ----------------------------------------------------------------------------
signals:
  void changed(); /**< The entries have changed, update the selector. */
  void setStatus(const QString&); /**< This is kab radio with the news... */
  void newFile(const QString&); /**< Notifies changes of the file name. */
  // ############################################################################

private: 
  class AddressBookPrivate;
  AddressBookPrivate *d;
};

#endif // ADDRESSBOOK_H

