/*  -*- C++ -*- */
#ifndef QCONFIGDB_H
#define QCONFIGDB_H

/* the Configuration Database library, Version II
 
   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$	 
*/

namespace std { };
using namespace std;

#include <list>
#include <map>
#include <qwidget.h>
#include <qcstring.h>
#include <qstrlist.h>

class QTimer;
class QDate;
class QString;
class QDateTime;

extern "C" {
#include <unistd.h>
}

class QTextStream;

/**
 * This is some STL interna, a function object for use with STL
 * container classes. Its only element function is the function
 * operator that returns a comparison value of the both objects
 * it is called with.
 */
struct QCStringLess 
  : public binary_function<const QCString&, const QCString&, bool> 
{
  /**
   * The function operator, inline.
   */
  bool operator()(const QCString& x, const QCString& y) const 
  { 
    return x < (const char*)y; // make one Qt operator fit exactly
  }
};

typedef map<QCString, QCString, QCStringLess> StringStringMap;

/**
 *  The class KeyValueMap is used for managing key-value-pairs
 *  WITHOUT any hierarchical structure.   Objects of it can be
 *  used as they are or in conjunction with the  configuration
 *  database class.
 *  While the first version used the string class, this second
 *  uses the QCString class.
 *  The class uses pairs of methods for each datatype, they are 
 *  called ::get and ::insert. Every overloaded version of this 
 *  methods get the key of the settings and a reference to the
 *  value to set or to store it in. A boolean result reports if
 *  there where errors or if the key already existed. Keys must
 *  of course be unique. Please note that the map does NOT store type 
 *  information for the keys. You may retrieve a boolean value for a string,
 *  it will work if the string is either "true" or "false".
 *  See the different get- and insert-methods for details.
 *
 *  Capabilities of the class are:
 *  <OL> 
 *  <LI> storing of any key-value-pair that is storable in 
 *    string values (no binary objects currently), </LI>
 *  <LI> key-value-pairs are saved in human-readable text files
 *    when saving to disk, </LI>
 *  <LI> the values may contain newline and tabulator characters
 *    which will still be there after saving and rereading, </LI>
 *  <LI> supports the following datatypes: <OL> 
 *    <LI> strings (of course), </LI>
 *    <LI> integers, </LI>
 *    <LI> floating point values and </LI>
 *    <LI> boolean states </LI> </OL> </LI> 
 *  <LI> supports storing and retrieving of lists of values of the 
 *    following datatypes: <OL> 
 *    <LI> strings, </LI>
 *    <LI> integers and </LI>
 *    <LI> floating point values </LI> </OL> 
 *    (boolean lists supported in future when requested) </LI> 
 *  <LI> easy syntax of files, in general it is supposed to be a 
 *    kind of guarantee  (you know that free software  never 
 *    guarantees anything, don't you?)  that every value that 
 *    has been  stored  by one of the member functions of the 
 *    class like <BR>
 *      <TT> insert(const QCString& key, [value&]); </TT> <BR>
 *    can also be retrieved using <BR>
 *      <TT> get(const QCString& key, [value&]);</TT> <BR>
 *    without being modified. <BR>
 *    (Please report anything that does not do so!) </LI> </OL> 
 *  The class is used to implement the #QConfigDB class.
 */

class KeyValueMap 
{
  // ############################################################################
protected:
  /**
   * A map storing the key-value-pairs.
   */
  StringStringMap* data;
  /**
   * Transform a complex string into a normal string object.
   * The values are not stored  as they are, they are coded into 
   * complex string where control and non-printable characters get a readable
   * representation.
   * When retrieving, this strings are translated back by this method.
   * \a orig contains the string read from the file, \a index the position from 
   * where to start the translation (need not be the beginning of the string), 
   * \a result contains the transformed string, \a noOfChars the number of 
   * characters used to parse the string. 
   * Returns true if there where no errors while parsing.
   * @see makeComplexString
   */
  bool parseComplexString(const QCString& orig, int index,
			  QCString& result, int& noOfChars) const; 
  /**
   * Codes a normal string into a complex string. 
   * @see parseComplexString
   */
  QCString makeComplexString(const QCString& orig); 
  /**
   * Inserts a complex string into the map.
   * The string must be coded already, no tests are performed. 
   * \a if force is false, an existing value will not be overridden.
   */
  bool insertRaw(const QCString& key, const QCString& value, bool force=false);
  /**
   * Retrieves the undecoded value (a complex string) of the given key.
   */
  bool getRaw(const QCString& key, QCString& value) const; 
  // ----------------------------------------------------------------------------
public:
  /**
   * The default constructor.
   */
  KeyValueMap(); 
  /**
   * The copy constructor.
   */
  KeyValueMap(const KeyValueMap&); 
  /**
   * The virtual destructor.
   */
  virtual ~KeyValueMap(); 
  /**
   * The begin iterator. Use it to iterate over the keys in the map.
   */
  StringStringMap::iterator begin();
  /**
   * The end iterator.
   */
  StringStringMap::iterator end();
  /**
   * Debugging aid: returns true if object is OK.
   */
  bool invariant();
  /**
   * Returns the number of key-value-pairs in the map.
   */
  unsigned int size() const; 
  /**
   * Delete all entries.
   */
  void clear(); 
  /**
   * Fills the map with the files contents.
   * If the parameter \a force is true, it overrides keys that are
   * already declared in the database and are declared again in the file.
   * If \a relax is true, the value of a string may be empty.
   */
  bool fill(const QString&, bool force=false, bool relax=false); 
  /**
   * Saves the database to a file. 
   * Only overrides existing files if force is true.
   */
  bool save(const QString&, bool force=false); 
  /**
   * Saves contents to an already open text stream.
   * \a count spaces are inserted before each line. This method is 
   * called when save hierarchical databases.
   * @see ::QConfigDB
   */
  bool save(QTextStream& file, int count); 
  /**
   * Get the value for the key as a string. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString& key, QCString& value) const;
  /**
   * Insert a string value for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString& key, const QCString& value, bool force=false);
  /**
   * Insert a character pointer  for the given key. 
   * pgcc treats character pointers as boolean objects, not as strings.
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  inline bool insert(const QCString& key, const char* value, bool force=false);
  /**
   * Insert a line like "key_a="Hallo!" into the map as a key-value-pair. 
   * If force is true existing keys will be overridden.
   * If relax is true the value may be empty an empty string.
   * If encode is false, the string will not be coded (do not use!).
   */
  bool insertLine(QCString, bool force=false, bool relax=false, bool encode=true);
  // ---------------
  /**
   * Get the value for the key as a long integer. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, long&) const;
  /**
   * Insert a long integer value for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const long&, bool force=false);
  // ---------------
  /**
   * For insertion of UNICODE strings, a special method pair is created. The 
   * data will be translated to utf8 and inserted in the map as a QCString. 
   * This will probably be not fast, but this methods are not suited to save 
   * large amounts of data. For saving anything else than UNICODE strings,
   * no such conversion is needed.
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, QString&) const;
  /**
   * Insert a UNICODE string value for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const QString&, bool force=false);
  // ---------------
  /**
   * Get the value for the key as a double. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, double&) const;
  /**
   * Insert a double value for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const double&, bool force=false);
  // ---------------
  /**
   * Get the value for the key as a boolean value. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, bool&) const;
  /**
   * Insert a boolean value for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const bool&, bool force=false);
  // ---------------
  /**
   * Get the value for the key as a list of strings. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, list<QCString>&) const;
  /**
   * Insert a list of strings for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const list<QCString>&, bool force=false);
  // --------------
  /**
   * Get the value for the key as a QStrList. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, QStrList&) const;
  /**
   * Insert a QStrList for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const QStrList&, bool force=false);
  // --------------
  /**
   * Get the value for the key as a QStringList. Beware of the difference -
   * a QStringList is a list of QString objects, while QStrList handles
   * char* like objects.
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, QStringList&) const;
  /**
   * Insert a QStringList for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const QStringList&, bool force=false);
  // --------------
  /**
   * Get the value for the key as a list of long integers. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, list<long>&) const;
  /**
   * Insert a list of long integers for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const list<long>&, bool force=false);
  // --------------
  /**
   * Get the value for the key as a list of integers. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, list<int>&) const;
  /**
   * Insert a list of integers for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const list<int>&, bool force=false);
  // -------------- some Qt high-level data types:
  /**
   * Get the value for the key as a QDate.
   * The value will be parsed to a integer list that must be a \e valid
   * date (see QDate documentation). \c false will be returned if the value
   * is not valid or a null date. This situation might only happen in
   * manually created files, since the insert-method for QDates rejects to
   * insert inalid dates, it inserts null dates instead.
   * @see get(const QCString&, QDate &)
   */
  bool get(const QCString&, QDate &) const;
  /**
   * Insert a QDate for the given key.
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   * \e Attention: If you insert an invalid date a null date will be used.
   * A null date will also be returned when retrieving this value.
   * You will not be able to store an invalid date and retrieve it using
   * ::get!
   */
  bool insert(const QCString&, const QDate&, bool force=false);
  // --------------
  /**
   * Get the value for the key as a list of doubles. 
   * \a key is the key to search for, \value is a reference to the object
   * the value for the key is assigned to.
   */
  bool get(const QCString&, list<double>&) const;
  /**
   * Insert a list of doubles for the given key. 
   * If force is true, an existing value for this key will be overridden.
   * The method returns false if the key exists and \a force is false.
   */
  bool insert(const QCString&, const list<double>&, bool force=false);
  // --------------
  // end of corresponding get-insert-pairs
  /**
   * Returns true if there are no keys declared in this map.
   */
  bool empty(); 
  /**
   * Erases all key-value-pairs in the map.
   */ 
  bool erase(const QCString& key); 
  // ############################################################################
};

/**
 * A Section object manages one section of a configuration database.
 * A configuration database consists of sections which in turn 
 * consist of other sections (recursive definition) and 
 * key-value-pairs. This file declares the Section class. An 
 * object of Section manages exactly one section during its 
 * lifetime.
 */

class Section
{
  // ############################################################################
public:
  /**
   * The StringSectionMap type is defined to make the code more readable.
   */
  typedef map<QCString, Section*, QCStringLess> StringSectionMap;
  // ----------------------------------------------------------------------------
protected:
  /**
   * A map containing the subsections of this section.
   */
  StringSectionMap sections;
  /**
   * The key-value-pairs of this section.
   */
  KeyValueMap keys;
  /**
   * The number of spaces a subsection is indented in text files.
   */
  static const int indent_width;
  /**
   * Insert the spaces for indention the lines of this section when saving.
   */
  void insertIndentSpace(QTextStream& file, int level);
  /**
   * Check whether the string (one line of the file currently read) marks the
   *  beginning of a new subsection (usually [sectionname]).
   */
  bool isBeginOfSection(QCString);
  /**
   * Check whether the string (one line of the file currently read) marks the
   *  end of a new subsection (usually [END]).
   */
  bool isEndOfSection(QCString);
  /**
   * Extract the name of the section from the string.
   * The string must contain the line that starts the section.
   * @see ::isBeginOfSection
   */
  QCString nameOfSection(const QCString&);
  // ----------------------------------------------------------------------------
public:
  /**
   * The default constructor.
   */
  Section();
  /**
   * Constructor that fills the keys with the given map entries.
   */
  Section(const KeyValueMap&);
  // handling sections:
  /**
   * Add an empty new section.
   */
  bool add(const QCString&);
  /**
   * Add the section.
   */
  bool add(const QCString&, Section*);
  /**
   * Search for the section, returning an iterator to it.
   */
  bool find(const QCString&, StringSectionMap::iterator&);
  /**
   * Search for the section, returning a pointer to the section object.
   */
  bool find(const QCString&, Section*&);
  /**
   * Remove this subsection.
   */
  bool remove(const QCString&);
  /**
   * Return the key-value-pairs of this (!) section.
   */
  KeyValueMap* getKeys();
  /**
   * Save this section to the given output stream.
   * Level is the position in section tree depth (the hierarchy level). 
   * It is used for indenting.
   */
  bool save(QTextStream& stream, int level=0);
  /**
   * Read one section from the given input stream.
   * The method does not expect the line that marks the begin of the 
   * section. If finish is false, the code does also not except the 
   * section to be ended with a line like [END].
   */
  bool readSection(QTextStream& file, bool finish=true);
  /**
   * Clears both subsections and keys.
   */
  bool clear();
  /**
   * Returns whether this section is empty. A section is empty if it has no
   * subsections and no key-value-pairs.
   */
  bool empty();
  // methods to allow iterating through the subsections
  /**
   * Return an iterator to the beginning of the subsections map.
   */
  StringSectionMap::iterator sectionsBegin();
  /**
   * Return an iterator to the end of the subsections map.
   */
  StringSectionMap::iterator sectionsEnd();
  /**
   * Return the number of subsections.
   */
  unsigned int noOfSections();
  // ############################################################################
};

/**
 *  The class QConfigDB is used to manage text-based data files 
 *  with hierarchical structure. <BR>
 *  It is derived from ::QWidget, so it may be derived to display 
 *  its contents. The basic implementation here does not display 
 *  anything to make it a lean class.  <BR>
 *  Some notes about the philosophy of the configuration 
 *  database library: <OL>
 *  <LI> The tasks in managing the structure are shared between the three
 *       involved classes ::KeyValueMap, ::Section and QConfigDB. </LI>
 *  <LI> \a QConfigDB
 *       is used for retrieving sections or key-value-maps from the data 
 *       hierarchy using keys. This keys are either pathes in UNIX style like
 *       "section_A/section_B/section_C", where C is a subsection of B which
 *       is in turn a subsection of A, or (STL) lists of strings in equivalent
 *       style (the first element of the list is treated as the first part of 
 *       the path, and so on). </LI>
 *  <LI> Section objects are used to manipulate the tree structure below one
 *       particular section. </LI>
 *  <LI> KeyValueMap objects are used to retrieve and modify the 
 *       key-value-pairs of one section, but not for its subsections. </LI> 
 *  </OL>
 *  Thus, to use the keys of a specific section in the database, you first
 *  retrieve it using the ::get methods, and then manipulate the 
 *  ::KeyValueMap you got. You may also retrieve a pointer to the whole 
 *  section, if you need access to its subsections, for example. Although
 *  this sounds complex, it is a really easy and comprehensive way to write
 *  code using tree-structured text files. <BR>
 *  See the code examples provided with the library for details.
 */

class QConfigDB : public QWidget
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
protected:
  /**
   * The toplevel section.
   */
  Section top;
  /**
   * A timer pointer for watching the file.
   */
  QTimer *timer;
  // ----------------------------------------------------------------------------
public:
  /**
   * The Qt standard constructor.
   */
  QConfigDB(QWidget* parent=0, const char* name=0);
  /**
   * The virtual destructor.
   */
  virtual ~QConfigDB();
  /**
   * Get the key-value-map for the section referenced by \a key.
   */
  bool get(const QCString& key, KeyValueMap*& map);
  /**
   * Get the key-value-map for the section referenced by \a key as key list.
   */
  bool get(const list<QCString>& key, KeyValueMap*& map);
  /**
   * Get the address of the specified Section object by its path. 
   * Never delete the section returned to you.
   */
  bool get(const QCString& key, Section*&);
  /**
   * Get the address of the specified Section object by a path list. 
   * Never delete the section returned to you.
   */  
  bool get(const list<QCString>& key, Section*&);
  /**
   * Get the keys of the toplevel section.
   */
  KeyValueMap* get();
  /**
   * Create the section with this path. 
   * All elements of the path that do not exist are created.
   */
  bool createSection(const QCString& key);
  /**
   * Create the section with a path like the path list. 
   * All elements of the path that do not exist are created.
   */
  bool createSection(const list<QCString>& key);
  /**
   * Load the file.
   * @see ::setFileName
   */
  bool load();
  /**
   * Save the file. 
   * \a header will be the comment in the first line of the file.
   * If \a force is \tt true, a file opened read-only will be switched
   * to read and write mode and back after saving. 
   * @see ::setFileName
   */
  bool save(const char* header=0, bool force=false); 
  /**
   * Set the current file name to \a name. 
   * Every QConfigDB object requires a file name to be set using
   * this method before the file operations work.
   * setFileName performs checks if the current user may use the file
   * in the requested way. If \a ro is true, she must have 
   * permissions to read the file, if it is false, permission must be
   * given to read and write the file. If \a mustexist is true, the file
   * must have existed before, if not, it might be created.
   * If any check failes, false is returned and the objects state is not
   * altered. Subsequent calls may be used to check if a file already 
   * exists.
   */
  bool setFileName(const QString& name, bool mustexist=true, bool ro=false);
  /**
   * Store the modification time of the file for later check of changes.
   */
  bool storeFileAge();
  /**
   * Give the current filename.
   */
  QString fileName(); 
  /**
   * Returns if the current file name is set for read only access.
   */
  bool isRO(); 
  /**
   * Clear the whole database.
   */
  bool clear();
  /**
   * Return whether the db is empty (e.g. the toplevel section is).
   */
  bool empty();
  /**
   * Return a string describing the version.
   */
  static const char* version() { return "2.0 $Revision$"; }
  /**
   * Check wether the given file is locked.
   * The method returns zero if not, a number > zero is the pid of the process
   * locking the file, a number < zero reports an error and indicates
   * that the file is locked.
   */
  static int IsLocked(const QString& fn);
  /**
   * Check an existing lock file for its validity.
   * \a fn is the name of the DATA file that is locked.
   * As lockfiles often remain when a program crashes, this function 
   * checks certain conditions that show that a lockfile is not in 
   * use anymore, these are:
   * ° there is no process with the pid in the lockfile,
   * ° the systems boot-time is after the creation of the lockfile.
   * The problem is that, if there is a process with the pid we have, 
   * this does not need to be the process that created the lockfile
   * the method returns only false if it is shure that no such process 
   * exists.
   * Returns false if the lockfile exists and is definitely stale or there 
   * is none, returns true if the lockfile seems to be really valid.
   */
  static bool CheckLockFile(const QString& filename);
  /**
   * The static method CleanLockFiles removes all files in the list 
   * ::LockFiles when called.
   * Thus this function should be installed as a handler for SIGINT, 
   * SIGQUIT, SIGKILL, SIGTERM and other program abortion signals or
   * should be called by the respective handlers.
   */
  static void CleanLockFiles(int);
  /**
   * Lock the current file.
   * Locking is done by creating a file <filename>.lock. 
   * QConfigDB-objects will reject opening a file for reading and 
   * writing if a lockfile for the  filename exists.
   */
  bool lock();
  /**
   * Unlock the file.
   */
  bool unlock();
  /**
   * If \a watch is <TT> true </TT> the object watches its file for changes. 
   * A timer is started that checks the file age every second and emits 
   * #fileChanged if it has been overridden meanwhile.
   */ 
  void watch(bool state);
  // ----------------------------------------------------------------------------
protected:
  /**
   * Transform a given path into a list of strings. 
   * All internal path handling is done with lists.
   */
  list<QCString> stringToKeylist(const QCString&);
  /**
   * The current filename.
   */
  QString filename; 
  /**
   * The current file opening mode.
   */
  bool readonly; 
  /**
   * Whether this object locked the file or not.
   */
  bool locked;
  /**
   * The modification time of the last file access.
   * Used to recognize file changes, is a null date if the modification time is 
   * unknown, what usually means that the current file has not been created and
   * does not exist by now. 
   * @see ::storeFileAge
   */
  QDateTime *mtime;
  /**
   * Lock the file.
   */
  bool lock(const QString& file);
  /**
   * Debugging aid, called from REQUIRE and ENSURE macros when the Nana library 
   * is used.
   */
  bool invariant();
  /**
   * All created lockfiles are notified in this list. 
   * The list contains the names of the lockfiles, not of the files itselfes.
   */
  static list<QString> LockFiles;
  // ----------------------------------------------------------------------------
public slots:
  /**
   * Check for file changes. 
   * This method returns true if the file has been changed on disk
   * after the last reading or saving.
   */
  bool checkFileChanged();
  // ----------------------------------------------------------------------------
signals:
  /**
   * This signal will be send when the database is cleared or reloaded.
   * The notification might be needed if pointers or iterators are stored 
   * outside the database object as they get invalid after reloading. 
   * The signal hands over its \a this pointer.
   */
  virtual void changed(QConfigDB*);
  /**
   * This signal will notify changes of the database <EM> file </EM>. The file
   * will be monitored on disk if #watch has been activated.
   */
  virtual void fileChanged();
  // ############################################################################
};

// ----- inline functions:
bool KeyValueMap::insert(const QCString& key, const char* value, bool force)
{ 
  return insert(key, (QCString)value, force); 
}
// -----

#endif // ! defined QCONFIGDB_H
