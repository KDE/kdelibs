/* This file is part of the KDE libraries
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KATE_DOCUMENT_H_
#define _KATE_DOCUMENT_H_

#include <QtCore/QLinkedList>
#include <QtCore/QMap>
#include <QtCore/QDate>
#include <QtGui/QClipboard>
#include <QtCore/QStack>

#include <kmimetype.h>
#include <kencodingprober.h>

#include <ktexteditor/document.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/variableinterface.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/rangefeedback.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/highlightinterface.h>

#include "katetextline.h"
#include "katenamespace.h"

namespace KTextEditor { class Plugin; class Attribute; }

namespace KIO { class TransferJob; }

class KateCodeFoldingTree;
class KateBuffer;
class KateView;
class KateSmartRange;
class KateLineInfo;
class KateBrowserExtension;
class KateDocumentConfig;
class KateHighlighting;
class KateSmartManager;
class KateUndoManager;
class KateEditHistory;
class KateOnTheFlyChecker;

class KateAutoIndent;
class KateTemplateHandler;

//
// Kate KTextEditor::Document class (and even KTextEditor::Editor ;)
//
class KateDocument : public KTextEditor::Document,
                     public KTextEditor::SessionConfigInterface,
                     public KTextEditor::ParameterizedSessionConfigInterface,
                     public KTextEditor::SearchInterface,
                     public KTextEditor::MarkInterface,
                     public KTextEditor::VariableInterface,
                     public KTextEditor::ModificationInterface,
                     public KTextEditor::ConfigInterface,
                     public KTextEditor::SmartInterface,
                     private KTextEditor::SmartRangeWatcher,
                     public KTextEditor::AnnotationInterface,
                     public KTextEditor::HighlightInterface
{
  Q_OBJECT
  Q_INTERFACES(KTextEditor::SessionConfigInterface)
  Q_INTERFACES(KTextEditor::ParameterizedSessionConfigInterface)
  Q_INTERFACES(KTextEditor::SearchInterface)
  Q_INTERFACES(KTextEditor::MarkInterface)
  Q_INTERFACES(KTextEditor::VariableInterface)
  Q_INTERFACES(KTextEditor::ModificationInterface)
  Q_INTERFACES(KTextEditor::SmartInterface)
  Q_INTERFACES(KTextEditor::AnnotationInterface)
  Q_INTERFACES(KTextEditor::ConfigInterface)
  Q_INTERFACES(KTextEditor::HighlightInterface)

  public:
    explicit KateDocument (bool bSingleViewMode=false, bool bBrowserView=false, bool bReadOnly=false,
                  QWidget *parentWidget = 0, QObject * = 0);
    ~KateDocument ();

    using ReadWritePart::closeUrl;
    bool closeUrl();

    KTextEditor::Editor *editor ();

    KTextEditor::Range rangeOnLine(KTextEditor::Range range, int line) const;

  /*
   * Overload this to have on-demand view creation
   */
  public:
    /**
     * @return The widget defined by this part, set by setWidget().
     */
    virtual QWidget *widget();

  public:
    bool readOnly () const { return m_bReadOnly; }
    bool browserView () const { return m_bBrowserView; }
    bool singleViewMode () const { return m_bSingleViewMode; }
    KateBrowserExtension *browserExtension () { return m_extension; }
    static bool simpleMode ();

  private:
    // only to make part work, don't change it !
    bool m_bSingleViewMode;
    bool m_bBrowserView;
    bool m_bReadOnly;
    KateBrowserExtension *m_extension;

  //
  // KTextEditor::Document stuff
  //
  public:
    KTextEditor::View *createView( QWidget *parent );
    const QList<KTextEditor::View*> &views () const;

    virtual KTextEditor::View* activeView() const { return m_activeView; }
    // Invalid covariant returns my a$$... for some reason gcc won't let me return a KateView above!
    KateView* activeKateView() const;

  Q_SIGNALS:
    void activeViewSelectionChanged(KTextEditor::View* view);

  private:
    QLinkedList<KateView*> m_views;
    QList<KTextEditor::View*> m_textEditViews;
    KTextEditor::View *m_activeView;

  //
  // KTextEditor::EditInterface stuff
  //
  public Q_SLOTS:
    virtual bool setText(const QString &);
    virtual bool setText(const QStringList& text);
    virtual bool clear ();

    virtual bool insertText ( const KTextEditor::Cursor &position, const QString &s, bool block = false );
    virtual bool insertText ( const KTextEditor::Cursor &position, const QStringList &text, bool block = false );

    virtual bool insertLine ( int line, const QString &s );
    virtual bool insertLines ( int line, const QStringList &s );

    bool removeText ( const KTextEditor::Range &range, bool block = false );
    bool removeLine ( int line );

    bool replaceText ( const KTextEditor::Range &range, const QString &s, bool block = false );

    // unhide method...
    bool replaceText (const KTextEditor::Range &r, const QStringList &l, bool b)
    { return KTextEditor::Document::replaceText (r, l, b); }

  public:
    virtual QString text ( const KTextEditor::Range &range, bool blockwise = false ) const;
    virtual QStringList textLines ( const KTextEditor::Range& range, bool block = false ) const;
    virtual QString text() const;
    virtual QString line(int line) const;
    virtual QChar character(const KTextEditor::Cursor& position) const;
    int lines() const;
    virtual KTextEditor::Cursor documentEnd() const;
    int numVisLines() const;
    int totalCharacters() const;
    int lineLength(int line) const;

  Q_SIGNALS:
    void charactersSemiInteractivelyInserted(const KTextEditor::Cursor& position, const QString& text);

  public:
//BEGIN editStart/editEnd (start, end, undo, cursor update, view update)
    /**
     * Enclose editor actions with @p editStart() and @p editEnd() to group
     * them.
     *
     * @param editSource the source for the grouped edit actions.
     */
    void editStart (Kate::EditSource editSource = Kate::NoEditSource);
    /**
     * Alias for @p editStart()
     */
    void editBegin (Kate::EditSource editSource = Kate::NoEditSource) { editStart(editSource); }
    /**
     * End a editor operation.
     * @see editStart()
     */
    void editEnd ();

    void pushEditState();
    void popEditState();

    bool startEditing () { editStart (Kate::ThirdPartyEdit); return true; }
    bool endEditing () { editEnd (); return true; }

//END editStart/editEnd

    void inputMethodStart();
    void inputMethodEnd();

//BEGIN LINE BASED INSERT/REMOVE STUFF (editStart() and editEnd() included)
    /**
     * Add a string in the given line/column
     * @param line line number
     * @param col column
     * @param s string to be inserted
     * @return true on success
     */
    bool editInsertText ( int line, int col, const QString &s, Kate::EditSource editSource = Kate::NoEditSource );
    /**
     * Remove a string in the given line/column
     * @param line line number
     * @param col column
     * @param len length of text to be removed
     * @return true on success
     */
    bool editRemoveText ( int line, int col, int len, Kate::EditSource editSource = Kate::NoEditSource );

    /**
     * Mark @p line as @p autowrapped. This is necessary if static word warp is
     * enabled, because we have to know whether to insert a new line or add the
     * wrapped words to the followin line.
     * @param line line number
     * @param autowrapped autowrapped?
     * @return true on success
     */
    bool editMarkLineAutoWrapped ( int line, bool autowrapped );

    /**
     * Wrap @p line. If @p newLine is true, ignore the textline's flag
     * KateTextLine::flagAutoWrapped and force a new line. Whether a new line
     * was needed/added you can grab with @p newLineAdded.
     * @param line line number
     * @param col column
     * @param newLine if true, force a new line
     * @param newLineAdded return value is true, if new line was added (may be 0)
     * @return true on success
     */
    bool editWrapLine ( int line, int col, bool newLine = true, bool *newLineAdded = 0 );
    /**
     * Unwrap @p line. If @p removeLine is true, we force to join the lines. If
     * @p removeLine is true, @p length is ignored (eg not needed).
     * @param line line number
     * @param removeLine if true, force to remove the next line
     * @return true on success
     */
    bool editUnWrapLine ( int line, bool removeLine = true, int length = 0 );

    /**
     * Insert a string at the given line.
     * @param line line number
     * @param s string to insert
     * @return true on success
     */
    bool editInsertLine ( int line, const QString &s, Kate::EditSource editSource = Kate::NoEditSource );
    /**
     * Remove a line
     * @param line line number
     * @return true on success
     */
    bool editRemoveLine ( int line, Kate::EditSource editSource = Kate::NoEditSource );

    bool editRemoveLines ( int from, int to, Kate::EditSource editSource = Kate::NoEditSource );

    /**
     * Remove a line
     * @param startLine line to begin wrapping
     * @param endLine line to stop wrapping
     * @return true on success
     */
    bool wrapText (int startLine, int endLine);
//END LINE BASED INSERT/REMOVE STUFF

  Q_SIGNALS:
    /**
     * Emmitted when text from @p line was wrapped at position pos onto line @p nextLine.
     */
    void editLineWrapped ( int line, int col, int len );

    /**
     * Emitted each time text from @p nextLine was upwrapped onto @p line.
     */
    void editLineUnWrapped ( int line, int col );

  public:
    KateUndoManager *undoManager();
    void undoSafePoint();

    bool undoDontMerge() const;
    void setUndoDontMerge(bool dontMerge);

    bool undoAllowComplexMerge() const;
    void setUndoAllowComplexMerge(bool allow);

    bool isEditRunning() const;

    void setMergeAllEdits(bool merge);

  private:
    int editSessionNumber;
    QStack<int> editStateStack;
    QStack<Kate::EditSource> m_editSources;
    bool editIsRunning;

  //
  // KTextEditor::UndoInterface stuff
  //
  public Q_SLOTS:
    void undo ();
    void redo ();

  public:
    uint undoCount () const;
    uint redoCount () const;
    KateEditHistory* history() const { return m_editHistory; }

  private:
    KateUndoManager* m_undoManager;
    KateEditHistory* m_editHistory;

  Q_SIGNALS:
    void undoChanged ();

  //
  // KTextEditor::SearchInterface stuff
  //
  public:
    QVector<KTextEditor::Range> searchText(
        const KTextEditor::Range & range,
        const QString & pattern,
        const KTextEditor::Search::SearchOptions options);

    KTextEditor::Search::SearchOptions supportedSearchOptions() const;

  private:
    /**
     * Return a widget suitable to be used as a dialog parent.
     */
    QWidget * dialogParent();

  /*
   * Access to the mode/highlighting subsystem
   */
  public:
    /**
     * Return the name of the currently used mode
     * \return name of the used mode
     *
     */
    virtual QString mode() const;

    /**
     * Return the name of the currently used mode
     * \return name of the used mode
     *
     */
    virtual QString highlightingMode() const;

    /**
     * Return a list of the names of all possible modes
     * \return list of mode names
     */
    virtual QStringList modes() const;

    /**
     * Return a list of the names of all possible modes
     * \return list of mode names
     */
    virtual QStringList highlightingModes() const;

    /**
     * Set the current mode of the document by giving its name
     * \param name name of the mode to use for this document
     * \return \e true on success, otherwise \e false
     */
    virtual bool setMode(const QString &name);

    /**
     * Set the current mode of the document by giving its name
     * \param name name of the mode to use for this document
     * \return \e true on success, otherwise \e false
     */
    virtual bool setHighlightingMode(const QString &name);
    /**
     * Returns the name of the section for a highlight given its index in the highlight
     * list (as returned by highlightModes()).
     * You can use this function to build a tree of the highlight names, organized in sections.
     * \param name the name of the highlight for which to find the section name.
     */
    virtual QString highlightingModeSection( int index ) const;

    /**
     * Returns the name of the section for a mode given its index in the highlight
     * list (as returned by modes()).
     * You can use this function to build a tree of the mode names, organized in sections.
     * \param name the name of the highlight for which to find the section name.
     */
    virtual QString modeSection( int index ) const;

  /*
   * SIGNALS
   * Following signals should be emitted by the document if the mode
   * or highlighting mode of the document changes
   */
  Q_SIGNALS:
    /**
     * Warn anyone listening that the current document's mode has
     * changed.
     *
     * \param document the document which's mode has changed
     */
    void modeChanged(KTextEditor::Document *document);

    /**
     * Warn anyone listening that the current document's highlighting mode has
     * changed.
     *
     * \param document the document which's mode has changed
     */
    void highlightingModeChanged(KTextEditor::Document *document);

  /*
   * Helpers....
   */
  public:
    void bufferHlChanged();

    /**
     * allow to mark, that we changed hl on user wish and should not reset it
     * atm used for the user visible menu to select highlightings
     */
    void setDontChangeHlOnSave();
    
    /**
     * Set that the BOM marker is forced via the tool menu
     */
    void bomSetByUser();
    
  //
  // KTextEditor::SessionConfigInterface and KTextEditor::ParameterizedSessionConfigInterface stuff
  //
  public:
    void readSessionConfig (const KConfigGroup&);
    void writeSessionConfig (KConfigGroup&);
    void readParameterizedSessionConfig (const KConfigGroup&, unsigned long configParameters);
    void writeParameterizedSessionConfig (KConfigGroup&, unsigned long configParameters);

  Q_SIGNALS:
    void configChanged();

  //
  // KTextEditor::MarkInterface
  //
  public Q_SLOTS:
    void setMark( int line, uint markType );
    void clearMark( int line );

    void addMark( int line, uint markType );
    void removeMark( int line, uint markType );

    void clearMarks();
    
    void requestMarkTooltip( int line, QPoint position );
    
    ///Returns true if the click on the mark should not be further processed
    bool handleMarkClick( int line );
    
    ///Returns true if the context-menu event should not further be processed
    bool handleMarkContextMenu( int line, QPoint position );

    void setMarkPixmap( MarkInterface::MarkTypes, const QPixmap& );

    void setMarkDescription( MarkInterface::MarkTypes, const QString& );

    void setEditableMarks( uint markMask );

  public:
    uint mark( int line );
    const QHash<int, KTextEditor::Mark*> &marks ();
    QPixmap markPixmap( MarkInterface::MarkTypes ) const;
    QString markDescription( MarkInterface::MarkTypes ) const;
    QColor markColor( MarkInterface::MarkTypes ) const;
    uint editableMarks() const;

  Q_SIGNALS:
    void markToolTipRequested( KTextEditor::Document* document, KTextEditor::Mark mark, QPoint position, bool& handled );
    
    void markContextMenuRequested( KTextEditor::Document* document, KTextEditor::Mark mark, QPoint pos, bool& handled );

    void markClicked( KTextEditor::Document* document, KTextEditor::Mark mark, bool& handled );
    
    void marksChanged( KTextEditor::Document* );
    void markChanged( KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction );

  private:
    QHash<int, KTextEditor::Mark*> m_marks;
    QHash<int,QPixmap>           m_markPixmaps;
    QHash<int,QString>           m_markDescriptions;
    uint                        m_editableMarks;

  //
  // KTextEditor::PrintInterface
  //
  public Q_SLOTS:
    bool printDialog ();
    bool print ();

  //
  // KTextEditor::DocumentInfoInterface ( ### unfinished )
  //
  public:
    /**
     * @return the name of the mimetype for the document.
     *
     * This method is using KMimeType::findByUrl, and if the pointer
     * is then still the default MimeType for a nonlocal or unsaved file,
     * uses mimeTypeForContent().
     */
    QString mimeType();

    /**
     * @return a pointer to the KMimeType for this document, found by analyzing the
     * actual content.
     *
     * Note that this method is *not* part of the DocumentInfoInterface.
     */
    KMimeType::Ptr mimeTypeForContent();

  //
  // KTextEditor::VariableInterface
  //
  public:
    QString variable( const QString &name ) const;

  Q_SIGNALS:
    void variableChanged( KTextEditor::Document*, const QString &, const QString & );

  private:
    QMap<QString, QString> m_storedVariables;

  //
  // KTextEditor::SmartInterface
  //
  public:
    virtual void clearSmartInterface();

    virtual int currentRevision() const;
    virtual void releaseRevision(int revision) const;
    virtual void useRevision(int revision = -1);
    virtual KTextEditor::Cursor translateFromRevision(const KTextEditor::Cursor& cursor, KTextEditor::SmartCursor::InsertBehavior insertBehavior = KTextEditor::SmartCursor::StayOnInsert) const;
    virtual KTextEditor::Range translateFromRevision(const KTextEditor::Range& range, KTextEditor::SmartRange::InsertBehaviors insertBehavior = KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight) const;

    virtual KTextEditor::SmartCursor* newSmartCursor(const KTextEditor::Cursor& position, KTextEditor::SmartCursor::InsertBehavior insertBehavior = KTextEditor::SmartCursor::MoveOnInsert);
    virtual void deleteCursors();

    virtual KTextEditor::SmartRange* newSmartRange(const KTextEditor::Range& range, KTextEditor::SmartRange* parent = 0L, KTextEditor::SmartRange::InsertBehaviors insertBehavior = KTextEditor::SmartRange::DoNotExpand);
    virtual KTextEditor::SmartRange* newSmartRange(KTextEditor::SmartCursor* start, KTextEditor::SmartCursor* end, KTextEditor::SmartRange* parent = 0L, KTextEditor::SmartRange::InsertBehaviors insertBehavior = KTextEditor::SmartRange::DoNotExpand);
    virtual void unbindSmartRange(KTextEditor::SmartRange* range);
    virtual void deleteRanges();

    // Syntax highlighting extension
    virtual void addHighlightToDocument(KTextEditor::SmartRange* topRange, bool supportDynamic);
    virtual const QList<KTextEditor::SmartRange*> documentHighlights() const;
    virtual void clearDocumentHighlights();

    virtual void addHighlightToView(KTextEditor::View* view, KTextEditor::SmartRange* topRange, bool supportDynamic);
    virtual void removeHighlightFromView(KTextEditor::View* view, KTextEditor::SmartRange* topRange);
    virtual const QList<KTextEditor::SmartRange*> viewHighlights(KTextEditor::View* view) const;
    virtual void clearViewHighlights(KTextEditor::View* view);

    // Action association extension
    virtual void addActionsToDocument(KTextEditor::SmartRange* topRange);
    virtual const QList<KTextEditor::SmartRange*> documentActions() const;
    virtual void clearDocumentActions();

    virtual void addActionsToView(KTextEditor::View* view, KTextEditor::SmartRange* topRange);
    virtual void removeActionsFromView(KTextEditor::View* view, KTextEditor::SmartRange* topRange);
    virtual const QList<KTextEditor::SmartRange*> viewActions(KTextEditor::View* view) const;
    virtual void clearViewActions(KTextEditor::View* view);

    KateSmartManager* smartManager() const { return m_smartManager; }

  Q_SIGNALS:
    void dynamicHighlightAdded(KateSmartRange* range);
    void dynamicHighlightRemoved(KateSmartRange* range);
  public Q_SLOTS:
    virtual void removeHighlightFromDocument(KTextEditor::SmartRange* topRange);
    virtual void removeActionsFromDocument(KTextEditor::SmartRange* topRange);

  protected:
    virtual void attributeDynamic(KTextEditor::Attribute::Ptr a);
    virtual void attributeNotDynamic(KTextEditor::Attribute::Ptr a);

  private:
    // Smart range watcher overrides
    virtual void rangeDeleted(KTextEditor::SmartRange* range);

    KateSmartManager* m_smartManager;
    QList<KTextEditor::SmartRange*> m_documentHighlights;
    QList<KTextEditor::SmartRange*> m_documentDynamicHighlights;
    QList<KTextEditor::SmartRange*> m_documentActions;

  //
  // Annotation Interface
  //
  public:

    virtual void setAnnotationModel( KTextEditor::AnnotationModel* model );
    virtual KTextEditor::AnnotationModel* annotationModel() const;

  Q_SIGNALS:
    void annotationModelChanged( KTextEditor::AnnotationModel*, KTextEditor::AnnotationModel* );

  private:
    KTextEditor::AnnotationModel* m_annotationModel;

  //
  // KParts::ReadWrite stuff
  //
  public:
    /**
     * open the file obtained by the kparts framework
     * the framework abstracts the loading of remote files
     * @return success
     */
    bool openFile ();

    /**
     * save the file obtained by the kparts framework
     * the framework abstracts the uploading of remote files
     * @return success
     */
    bool saveFile ();

    void setReadWrite ( bool rw = true );

    void setModified( bool m );

  private:
    void activateDirWatch (const QString &useFileName = QString());
    void deactivateDirWatch ();

    QString m_dirWatchFile;

  public:
    /**
     * Type chars in a view
     */
    bool typeChars ( KateView *type, const QString &chars );

    /**
     * gets the last line number (lines() - 1)
     */
    inline int lastLine() const { return lines()-1; }

    // Repaint all of all of the views
    void repaintViews(bool paintOnlyDirty = true);

    KateHighlighting *highlight () const;

  public Q_SLOTS:    //please keep prototypes and implementations in same order
    void tagLines(int start, int end);
    void tagLines(KTextEditor::Cursor start, KTextEditor::Cursor end);

  //export feature, obsolute
  public Q_SLOTS:
     void exportAs(const QString&) { }

  Q_SIGNALS:
    void preHighlightChanged(uint);

  private Q_SLOTS:
    void internalHlChanged();

  public:
    void addView(KTextEditor::View *);
    /** removes the view from the list of views. The view is *not* deleted.
     * That's your job. Or, easier, just delete the view in the first place.
     * It will remove itself. TODO: this could be converted to a private slot
     * connected to the view's destroyed() signal. It is not currently called
     * anywhere except from the KateView destructor.
     */
    void removeView(KTextEditor::View *);
    void setActiveView(KTextEditor::View*);

    bool ownedView(KateView *);

    uint toVirtualColumn( const KTextEditor::Cursor& );
    void newLine( KateView*view ); // Changes input
    void backspace(     KateView *view, const KTextEditor::Cursor& );
    void del(           KateView *view, const KTextEditor::Cursor& );
    void transpose(     const KTextEditor::Cursor& );

    void paste ( KateView* view, QClipboard::Mode = QClipboard::Clipboard );

  public:
    void indent ( KateView *view, uint line, int change );
    void comment ( KateView *view, uint line, uint column, int change );
    void align ( KateView *view, const KTextEditor::Range &range );

    enum TextTransform { Uppercase, Lowercase, Capitalize };

    /**
      Handling uppercase, lowercase and capitalize for the view.

      If there is a selection, that is transformed, otherwise for uppercase or
      lowercase the character right of the cursor is transformed, for capitalize
      the word under the cursor is transformed.
    */
    void transform ( KateView *view, const KTextEditor::Cursor &, TextTransform );
    /**
      Unwrap a range of lines.
    */
    void joinLines( uint first, uint last );

  private:
    bool removeStringFromBeginning(int line, const QString &str);
    bool removeStringFromEnd(int line, const QString &str);

    /**
      Find the position (line and col) of the next char
      that is not a space. If found line and col point to the found character.
      Otherwise they have both the value -1.
      @param line Line of the character which is examined first.
      @param col Column of the character which is examined first.
      @return True if the specified or a following character is not a space
               Otherwise false.
    */
    bool nextNonSpaceCharPos(int &line, int &col);

    /**
      Find the position (line and col) of the previous char
      that is not a space. If found line and col point to the found character.
      Otherwise they have both the value -1.
      @return True if the specified or a preceding character is not a space.
               Otherwise false.
    */
    bool previousNonSpaceCharPos(int &line, int &col);

    /**
    * Sets a comment marker as defined by the language providing the attribute
    * @p attrib on the line @p line
    */
    void addStartLineCommentToSingleLine(int line, int attrib=0);
    /**
    * Removes a comment marker as defined by the language providing the attribute
    * @p attrib on the line @p line
    */
    bool removeStartLineCommentFromSingleLine(int line, int attrib=0);

    /**
    * @see addStartLineCommentToSingleLine.
    */
    void addStartStopCommentToSingleLine(int line, int attrib=0);
    /**
    *@see removeStartLineCommentFromSingleLine.
    */
    bool removeStartStopCommentFromSingleLine(int line, int attrib=0);
    /**
    *@see removeStartLineCommentFromSingleLine.
    */
    bool removeStartStopCommentFromRegion(const KTextEditor::Cursor &start, const KTextEditor::Cursor &end, int attrib=0);

    /**
     * Add a comment marker as defined by the language providing the attribute
     * @p attrib to each line in the selection.
     */
    void addStartStopCommentToSelection( KateView *view, int attrib=0 );
    /**
     * @see addStartStopCommentToSelection.
     */
    void addStartLineCommentToSelection( KateView *view, int attrib=0 );

    /**
     * Removes comment markers relevant to the language providing
     * the attribuge @p attrib from each line in the selection.
     *
     * @return whether the operation succeeded.
     */
    bool removeStartStopCommentFromSelection( KateView *view, int attrib=0 );
    /**
     * @see removeStartStopCommentFromSelection.
     */
    bool removeStartLineCommentFromSelection( KateView *view, int attrib=0 );

  public:
    QString getWord( const KTextEditor::Cursor& cursor );

  public:
    void newBracketMark( const KTextEditor::Cursor& start, KTextEditor::Range& bm, int maxLines = -1 );
    bool findMatchingBracket( KTextEditor::Range& range, int maxLines = -1 );

  private:
    void guiActivateEvent( KParts::GUIActivateEvent *ev );

  public:
    const QString &documentName () const { return m_docName; }

    void setDocName (QString docName);

    void lineInfo (KateLineInfo *info, unsigned int line);

    KateCodeFoldingTree *foldingTree ();

  public:
    /**
     * @return wheather the document is modified on disk since last saved.
     */
    bool isModifiedOnDisc() { return m_modOnHd; }

    void setModifiedOnDisk( ModifiedOnDiskReason reason );

    void setModifiedOnDiskWarning ( bool on );

  public Q_SLOTS:
    /**
     * Ask the user what to do, if the file has been modified on disk.
     * Reimplemented from KTextEditor::Document.
     */
    void slotModifiedOnDisk( KTextEditor::View *v = 0 );

    /**
     * Reloads the current document from disk if possible
     */
    bool documentReload ();

    bool documentSave ();
    bool documentSaveAs ();

    virtual bool save();
  public:
    virtual bool saveAs( const KUrl &url );
  private:
    bool m_saveAs;
  Q_SIGNALS:
    /**
     * Indicate this file is modified on disk
     * @param doc the KTextEditor::Document object that represents the file on disk
     * @param isModified indicates the file was modified rather than created or deleted
     * @param reason the reason we are emitting the signal.
     */
    void modifiedOnDisk (KTextEditor::Document *doc, bool isModified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason);

  public:
    void ignoreModifiedOnDiskOnce();

  private:
    int m_isasking; // don't reenter slotModifiedOnDisk when this is true
                    // -1: ignore once, 0: false, 1: true

  public:
    bool setEncoding (const QString &e);
    const QString &encoding() const;
    void setProberTypeForEncodingAutoDetection (KEncodingProber::ProberType);
    KEncodingProber::ProberType proberTypeForEncodingAutoDetection() const;


  public Q_SLOTS:
    void setWordWrap (bool on);
    void setWordWrapAt (uint col);

  public:
    bool wordWrap() const;
    uint wordWrapAt() const;

  public Q_SLOTS:
    void setPageUpDownMovesCursor(bool on);

  public:
    bool pageUpDownMovesCursor() const;

   // code folding
  public:
    uint getRealLine(unsigned int virtualLine);
    uint getVirtualLine(unsigned int realLine);
    uint visibleLines ();
    KateTextLine::Ptr kateTextLine(uint i);
    KateTextLine::Ptr plainKateTextLine(uint i);

  Q_SIGNALS:
    void codeFoldingUpdated();
    void aboutToRemoveText(const KTextEditor::Range&);
    void textRemoved();

  private Q_SLOTS:
    void slotModOnHdDirty (const QString &path);
    void slotModOnHdCreated (const QString &path);
    void slotModOnHdDeleted (const QString &path);

  private:
    /**
     * create a MD5 digest of the file, if it is a local file,
     * and fill it into the string @p result.
     * This is using KMD5::hexDigest().
     *
     * @return wheather the operation was attempted and succeeded.
     */
    bool createDigest ( QByteArray &result );

    /**
     * create a string for the modonhd warnings, giving the reason.
     */
    QString reasonedMOHString() const;

    /**
     * Removes all trailing whitespace form @p line, if
     * the cfRemoveTrailingDyn confg flag is set,
     * and the active view cursor is not on line and behind
     * the last nonspace character.
     */
    void removeTrailingSpace(int line);
    inline void blockRemoveTrailingSpaces(bool block)
    { m_blockRemoveTrailingSpaces = block; }

  private:
    /// if true, removeTrailingSpace() returns immediately.
    bool m_blockRemoveTrailingSpaces;

  public:
    void updateFileType (const QString &newType, bool user = false);

    QString fileType () const { return m_fileType; }

  //
  // REALLY internal data ;)
  //
  private:
    // text buffer
    KateBuffer *m_buffer;

    // indenter
    KateAutoIndent *const m_indenter;

    bool hlSetByUser;
    bool m_bomSetByUser;

    bool m_modOnHd;
    ModifiedOnDiskReason m_modOnHdReason;
    QByteArray m_digest; // MD5 digest, updated on load/save

    QString m_docName;
    int m_docNameNumber;

    // file type !!!
    QString m_fileType;
    bool m_fileTypeSetByUser;

    /**
     * document is still reloading a file
     */
    bool m_reloading;

  public Q_SLOTS:
    void slotQueryClose_save(bool *handled, bool* abortClosing);

  public:
    virtual bool queryClose();

    void makeAttribs (bool needInvalidate = true);

    static bool checkOverwrite( KUrl u, QWidget *parent );

  /**
   * Configuration
   */
  public:
    KateDocumentConfig *config() { return m_config; }
    KateDocumentConfig *config() const { return m_config; }

    void updateConfig ();

  private:
    KateDocumentConfig *m_config;

  /**
   * Variable Reader
   * TODO add register functionality/ktexteditor interface
   */
  private:
    /**
     * read dir config file
     */
    void readDirConfig ();

    /**
      Reads all the variables in the document.
      Called when opening/saving a document
    */
    void readVariables(bool onlyViewAndRenderer = false);

    /**
      Reads and applies the variables in a single line
      TODO registered variables gets saved in a [map]
    */
    void readVariableLine( QString t, bool onlyViewAndRenderer = false );
    /**
      Sets a view variable in all the views.
    */
    void setViewVariable( QString var, QString val );
    /**
      @return weather a string value could be converted
      to a bool value as supported.
      The value is put in *result.
    */
    static bool checkBoolValue( QString value, bool *result );
    /**
      @return weather a string value could be converted
      to a integer value.
      The value is put in *result.
    */
    static bool checkIntValue( QString value, int *result );
    /**
      Feeds value into @p col using QColor::setNamedColor() and returns
      wheather the color is valid
    */
    static bool checkColorValue( QString value, QColor &col );

    /**
     * helper regex to capture the document variables
     */
    static QRegExp kvLine;
    static QRegExp kvLineWildcard;
    static QRegExp kvLineMime;
    static QRegExp kvVar;

    bool s_fileChangedDialogsActivated;

  //
  // KTextEditor::ConfigInterface
  //
  public:
     QStringList configKeys() const;
     QVariant configValue(const QString &key);
     void setConfigValue(const QString &key, const QVariant &value);

  // TemplateInterface
  public:
    /**
     * Starts a KateTemplateHandler for the given template. Multiple handlers may
     * run at the same time, while only the last one created for this document
     * will be active.
     *
     * \see KTextEditor::TemplateInterface::insertTemplateTextImplementation(), KateTemplateHandler
     */
    virtual bool insertTemplateTextImplementation ( const KTextEditor::Cursor &c, const QString &templateString,
                                                    const QMap<QString,QString> &initialValues, QWidget *);

  //
  // KTextEditor::HighlightInterface
  //
  public:
    virtual KTextEditor::Attribute::Ptr defaultStyle(const KTextEditor::HighlightInterface::DefaultStyle ds) const;
    virtual QList< KTextEditor::HighlightInterface::AttributeBlock > lineAttributes(const unsigned int line);
    virtual QStringList embeddedHighlightingModes() const;
    virtual QString highlightingModeAt(const KTextEditor::Cursor& position);

  protected Q_SLOTS:
      void dumpRegionTree();

  private slots:
      void slotCompleted();
      void slotCanceled();

  private:
      class LoadSaveFilterCheckPlugins;

  private:
      bool m_savingToUrl;
      void setPreSavePostDialogFilterChecks(QStringList plugins) {m_preSavePostDialogFilterChecks=plugins;}
      QStringList m_preSavePostDialogFilterChecks;
      void setPostSaveFilterChecks(QStringList plugins) {m_postSaveFilterChecks=plugins;}
      QStringList m_postSaveFilterChecks;
      void setPostLoadFilterChecks(QStringList plugins) {m_postLoadFilterChecks=plugins;}
      QStringList m_postLoadFilterChecks;
      static LoadSaveFilterCheckPlugins* loadSaveFilterCheckPlugins();

  public:
      QString defaultDictionary() const;
      QList<QPair<KTextEditor::SmartRange*, QString> > dictionaryRanges() const;
      bool isOnTheFlySpellCheckingEnabled() const;

      QPair<KTextEditor::Range, QString> onTheFlyMisspelledItem(const KTextEditor::Cursor &cursor) const;
      QString dictionaryForMisspelledRange(const KTextEditor::Range& range) const;
      void clearMisspellingForWord(const QString& word);

  public Q_SLOTS:
      void clearDictionaryRanges();
      void setDictionary(const QString& dict, const KTextEditor::Range &range);
      void revertToDefaultDictionary(const KTextEditor::Range &range);
      void setDefaultDictionary(const QString& dict);
      void onTheFlySpellCheckingEnabled(bool enable);
      void respellCheckBlock(int start, int end) {respellCheckBlock(this,start,end);}
      void refreshOnTheFlyCheck(const KTextEditor::Range &range = KTextEditor::Range::invalid());

  protected Q_SLOTS:
      void dictionaryRangeEliminated(KTextEditor::SmartRange *smartRange);
      void deleteDiscardedSmartRanges();

  Q_SIGNALS:
      void respellCheckBlock(KateDocument *document,int start, int end);
      void dictionaryRangesPresent(bool yesNo);
      void defaultDictionaryChanged(KateDocument *document);

  public:
      bool containsCharacterEncoding(const KTextEditor::Range& range);

      typedef QList<QPair<int, int> > OffsetList;

      int computePositionWrtOffsets(const OffsetList& offsetList, int pos);

      /**
       * The first OffsetList is from decoded to encoded, and the second OffsetList from
       * encoded to decoded.
       **/
      QString decodeCharacters(const KTextEditor::Range& range,
                               KateDocument::OffsetList& decToEncOffsetList,
                               KateDocument::OffsetList& encToDecOffsetList);
      void replaceCharactersByEncoding(const KTextEditor::Range& range);

      enum EncodedCharaterInsertionPolicy {EncodeAlways, EncodeWhenPresent, EncodeNever};

  protected:
      KateOnTheFlyChecker *m_onTheFlyChecker;
      QString m_defaultDictionary;
      QList<QPair<KTextEditor::SmartRange*, QString> > m_dictionaryRanges;
      QList<KTextEditor::SmartRange*> m_discardedSmartRanges;
      KTextEditor::SmartRangeNotifier *m_dictionaryRangeNotifier;

      KTextEditor::SmartRangeNotifier* dictionaryRangeNotifier();
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

