#ifndef __kregexpeditorinterface_h__
#define __kregexpeditorinterface_h__

/**
 *  A widget for terminal emulation, interpreting vt102
 * 
 * @author Jesper K. Pedersen blackie@kde.org
 *
 * The actual widget is located in kdebase/konsole, with an interface 
 * in kdelibs. This means that it is a bit more comlicated to create
 * an instance of it, but only a little bit more complicated. 
 *
 * To check if Terminal in kdeutils is installed and available use
 * this line: 
 * 
 * <pre>
 * bool installed=!KTrader::self()->query("TerminalEmulation/TerminalEmulation").isEmpty();
 * </pre>
 *
 * The following is a template for what you need to do to create an
 * instance of the regular expression dialog:
 *
 * <pre>
 * QDialog *termDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "TerminalEmulation/TerminalEmulation" );
 * if ( editorDialog ) {
 *   // konsole was installed, so the dialog was found fetch the
 *   // editor interface 
 *   TermEmuInterface* term = static_cast<TermEmuInterface*>( termDialog->qt_cast( "TermEmuInterface" ) );
 *   Q_ASSERT( term ); // This should not fail!
 *   
 *   // now use it.
 *   KProcess* proc = new KProcess;
 *   proc << "ls" << "-l" << "/";
 *   term->setProcess( proc );
 *   proc->start();
 *
 *   // Finally show the dialog
 *   term->show();
 * }
 * else {
 *   // annoy the user with an error message, telling him to install
 *   // konsole... 
 * }
 * </pre>
 *
 * Note: signals and slots must be connected to the termDialog object,
 * not to the term object: 
 * <pre>
 * connect( this, SIGNAL( setReadOnly( bool ) ), termDialog, SLOT( setInteractive( bool ) ) );
 * </pre>
 *
 */
class TermEmuInterface
{
signals:
  /**
   * This signal is emitted when the widget's size changes...
   */
  virtual void sizeChanged( int width, int height ) = 0;
  
  /**
   * This signal is emitted when a line is being added to the widget.
   * You can change the text being shown, colorize it etc.
   * NOT SUPPORTED YET
   */
//   virtual void addingLine( charData* line );
public:
  /**
   * set whether the cursor is shown...
   * @see isCursorShown()
   */
  virtual void setCursorShown( const bool ) = 0;

  /**
   * whether the cursor is shown...
   * @see setCursorShown()
   */
  virtual bool isCursorShown() const = 0;

  /**
   * allow the user to give input or not...
   * @see isInteractive()
   */
  virtual void setInteractive( const bool ) = 0;

  /**
   * is the user allowed to give input ?
   * @see setInteractive()
   */
  virtual bool isInteractive() const = 0;

  /**
   * set the process we're emulating a Terminal for...
   */
  virtual void setProcess( KProcess* proc ) = 0;
};

#endif

