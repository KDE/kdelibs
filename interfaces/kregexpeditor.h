#ifndef kregexpeditor_h
#define kregexpeditor_h

#include <qstring.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qvaluestack.h>
#include <qcstring.h>

class KRegExpEditorPrivate;

/**
   Regular Expression Editor.

   @author Jesper Kjær Pedersen <blackie@kde.org> 
   @version 0.1
 **/
class KRegExpEditor  :public QWidget
{
Q_OBJECT

public:
  static KRegExpEditor* createEditor( QWidget* parent, const char* name = 0);
  virtual QString regexp() = 0;
  virtual bool canUndo() const = 0;
  virtual bool canRedo() const = 0;

  //  static const QString version;

protected:
  KRegExpEditor( QWidget *parent, const char *name = 0 );

public slots:
  virtual void slotRedo() = 0;
  virtual void slotUndo() = 0;

};

#endif

