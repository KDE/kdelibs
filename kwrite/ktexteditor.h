#ifndef __ktexteditor_h__
#define __ktexteditor_h__

#include <kparts/part.h>
#include <qwidget.h>

namespace KTextEditor
{

class Document;

class View : public QWidget, public KXMLGUIClient
{
  Q_OBJECT
public:
  View( Document *doc, QWidget *parent, const char *name = 0 );
  virtual ~View();

  virtual Document *textDocument() const; // XXX fix when renaming KXMLGUIClient::document

  virtual void setCursorPosition( int line, int col, bool mark = false ) = 0;
  virtual void getCursorPosition( int *line, int *col ) = 0;

  virtual void setSelection( int row_from, int col_from, int row_to, int col_t ) = 0;
  virtual bool hasSelection() const = 0;
  virtual QString selection() const = 0;

  /**
   * You should reimplement this method.
   * If the internal popupmenu property is enabled, then the implementation is free
   * to handle/use/implement/show a context popupmenu ( see also KContextMenuManager class
   * in kdeui ) . If disabled, then the implementation should emit the @ref contextPopupMenu
   * signal.
   */
  virtual void setInternalContextMenuEnabled( bool b );
  virtual bool internalContextMenuEnabled() const;

signals:
  void contextPopupMenu( const QPoint &p );

private:
  class ViewPrivate;
  ViewPrivate *d;
};

class Document : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  Document( QObject *parent = 0, const char *name = 0 );
  virtual ~Document();

  virtual View *createView( QWidget *parent, const char *name ) = 0;

  virtual QList<View> views() const;

  virtual QString textLine( int line ) const = 0;
  virtual int numLines() const = 0;

  virtual void insertLine( const QString &s, int line = -1 ) = 0;
  virtual void insertAt( const QString &s, int line, int col, bool mark = FALSE ) = 0;
  virtual void removeLine( int line ) = 0;

  virtual QString text() const = 0;
  virtual int length() const = 0;

  virtual bool isOverwriteMode() const = 0;

public slots:
  virtual void setText( const QString &t ) = 0;
  virtual void setOverwriteMode( bool b ) = 0;
signals:
  void textChanged();

protected:
  virtual void addView( View *view );
  virtual void removeView( View *view );

private slots:
  void slotViewDestroyed();

private:
  class DocumentPrivate;
  DocumentPrivate *d;
};

};

#endif
