/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __kpanelextension_h__
#define __kpanelextension_h__

class KConfig;
class KPanelExtensionPrivate;

#include <qframe.h>

/**
 * Implements a panel extension.
 *
 * Panel extensions
 *   @li Are small applications living in the Window Manager dock managed by the panel.
 *   @li Are implemented as DSOs (Dynamic Shared Objects).
 *
 * Note: For security and stability reasons the panel won't load
 * untrusted third party extensions directly into its namespace but via an
 * external wrapper process.
 *
 * The panel locates available extensions by searching for extension desktop
 * files in (ALL_KDEDIRS)/share/apps/kicker/extensions. Every panel extension should
 * install a desktop file there to be recognised by the panel.
 *
 * Besides standard keys like "Name", "Comment" and "Icon" there are
 * two panel extension specific keys:
 *
 * @sect X-KDE-Library
 *
 * Used by the panel to locate the extension DSO (Dynamic Shared Object)
 * Example: X-KDE-Library=libexampleextension
 *
 * @sect X-KDE-UniqueExtension
 *
 * Similar to @ref KApplication and @ref KUniqueApplication there are
 * two types of panel extensions. Use unique extensions when it makes no
 * sence to run more than one instance of an extension in the panel. A
 * good example for unique extensions is the taskbar extension.  Use normal
 * extensions when you need instance specific configuration. An example
 * is a subpanel extension where you might want to run more than one instances.
 * X-KDE-UniqueExtension is a boolean key which defaults
 * to "false".  Example: X-KDE-UniqueExtension=true
 *
 * Back to panel extension DSOs, the following conventions are used for KDE:
 * Name:    lib<extensionname>extension.la
 * LDFLAGS: -module -no-undefined
 *
 * To implement a panel extension it is not enough to write a class
 * inheriting from KPanelExtension but you also have to provide a
 * factory function in your DSO.  A sample factory function could look
 * like this:
 *
 * <pre>
 *
 * extern "C"
 * {
 *     KPanelExtension* init(QWidget *parent, const QString& configFile)
 *     {
 *         KGlobal::locale()->insertCatalogue("exampleextension");
 *         return new ExampleExtension(configFile, KPanelExtension::Normal,
 *                       KPanelExtension::About | KPanelExtension::Help | KPanelExtension::Preferences,
 *                       parent, "exampleextension");
 *     }
 * }
 *
 * </pre>
 *
 * Note: Don't change the factory function signature or the panel will
 * fail to load your extension.
 *
 * @author Matthias Elter <elter@kde.org>
 * @short KDE Panel Extension class
 **/
class KPanelExtension : public QFrame
{
    Q_OBJECT

public:

    enum Type { Normal = 0, Stretch };
    enum Action { About = 1, Help = 2, Preferences = 4, ReportBug = 8 };
    enum Position { Left = 0, Right, Top, Bottom };
    enum Alignment { LeftTop = 0, Center, RightBottom };
    enum Size { SizeTiny = 0, SizeSmall, SizeNormal, SizeLarge, SizeCustom };

    /**
     * Constructs a KPanelExtension just like any other widget.
     *
     * @param configFile The configFile handed over in the factory function.
     * @param Type The extension @ref type().
     * @param actions Standard RMB menu actions supported by the extension (see @ref action() ).
     * @param parent The pointer to the parent widget handed over in the factory function.
     * @param name A Qt object name for your extension.
     **/
    KPanelExtension(const QString& configFile, Type t = Normal,
		 int actions = 0, QWidget *parent = 0, const char *name = 0);

    /**
     * Destructor.
     **/
    ~KPanelExtension();

    /**
     * Returns the prefered size for a given Position.
     *
     * Every extension should reimplement this function.
     *
     * Depending on the panel position the extensions can choose a prefered size for that
     * location in the Window Manager Dock. Please note that the size can not be larger than the
     * maxsize given by the handler.
     **/
    virtual QSize sizeHint(Position /*p*/, QSize maxsize) const { return maxsize; }

    /**
     * Always use this @ref KConfig object to save/load your extensions configuration.
     *
     * For unique extensions this config object will write to a config file called
     * <extensionname>rc in the users local KDE directory.
     *
     * For normal extensions this config object will write to a instance specific config file
     * called <extensionname><instanceid>rc in the users local KDE directory.
     **/
    KConfig* config() const { return _config; }

    /**
     * @return Type indicating the extensions type.
     * @ref Type
     **/
    Type type() const { return _type; }

    /**
     * @return int indicating the supported RMB menu actions.
     * @ref Action
     **/
    int actions() const { return _actions; }

    /**
     * Generic action dispatcher. Called  when the user selects an item
     * from the extensions RMB menu.
     *
     * Reimplement this function to handle actions.
     *
     * For About, Help, Preferences and ReportBug use the convenience handlers
     * ref about(), help(), preferences(), reportBug()
     *
     **/
    virtual void action( Action a );


    /**
     * Reimplement this function to set a prefered dock position for your extension.
     * The extension manager will try to place new instances of this extension according
     * to this setting.
     * @return Position
     **/
    virtual Position preferedPosition() const { return Bottom; }

    /**
     * @internal
     **/
    void setPosition( Position p );
    /**
     * @internal
     **/
    void setAlignment( Alignment a );
    /**
     * @internal
     **/
    void setSize( Size size, int customSize );

    /**
    * @return the extension's size
    **/
    Size sizeSetting() const;

    /**
    * @return the custom sizel setting in pixels
    **/
    int customSize() const;
                                                             

signals:
    /**
     * Emit this signal to make the panel relayout all extensions in the dock, when you want
     * to change the extensions size.
     * The panel is going to relayout all extensions based on their prefered size.
     **/
    void updateLayout();

protected:

    /**
     * Is called when the user selects "About" from the extensions RMB menu.
     * Reimplement this function to launch a about dialog.
     *
     * Note that this is called only when your extension supports the About action.
     * See @ref Action.
     **/
    virtual void about() {}

    /**
     * Is called when the user selects "Help" from the extensions RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your extension supports the Help action.
     * See @ref Action.
     **/
    virtual void help() {}

    /**
     * Is called when the user selects "Preferences" from the extensions RMB menu.
     * Reimplement this function to launch a preferences dialog or kcontrol module.
     *
     * Note that this is called only when your extension supports the preferences action.
     * See @ref Action.
     **/
    virtual void preferences() {}
	
    /**
     * Is called when the user selects "Report bug" from the applet's RMB menu.
     * Reimplement this function to launch a bug reporting dialog.
     *
     * Note that this is called only when your applet supports the ReportBug
     * action.
     * See @ref Action.
     **/
    virtual void reportBug() {}

    /**
     * @return the extension's position. (left, right, top, bottom)
     **/
    Position position() const { return _position; }

    /**
     * @return the extension's alignment. (left/top, center, or right/bottom)
     **/
    Alignment alignment() const { return _alignment; }

    /**
     * @return the extensions orientation. (horizontal or vertical)
     **/
    Orientation orientation();

    /**
     * @return the appropriate size in pixels for the panel
     */
    int sizeInPixels() const;

    /**
     * This extension has changed its position.
     * Reimplement this change handler in order to adjust the look of your
     * applet.
     **/
    virtual void positionChange( Position ) {};

    /**
     * This extension has changed its alignment.
     * Reimplement this change handler in order to adjust the look of your
     * applet.
     **/
    virtual void alignmentChange( Alignment ) {};

private:
    Type         		_type;
    Position     		_position;
    Alignment    		_alignment;
    KConfig*     		_config;
    int          		_actions;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KPanelExtensionPrivate     *d;
};

#endif
