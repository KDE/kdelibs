#ifndef kregexpeditor_h
#define kregexpeditor_h

class QWidget;
class QDialog;

/**
   Regular Expression Editor.

   @author Jesper Kjær Pedersen <blackie@kde.org> 
   @version 0.1
 **/
class KRegExpEditor  
{
public:
  static QWidget* createEditor( QWidget* parent, const char* name = 0);
  static QDialog* createDialog( QWidget* parent, const char* name = 0);
};

#endif

