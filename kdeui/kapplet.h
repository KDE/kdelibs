/* This file is part of the KDE libraries

    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KAPPLET_H
#define KAPPLET_H

#include <qwidget.h>
#include <dcopobject.h>

class KAppletData;

/**
* KDE Panel Applet class
*
* This class implements panel applets.
*
* @author Matthias Ettrich <ettrich@kde.org>
* @short KDE Panel Applet class
*/
class KApplet : public QWidget, DCOPObject
{
    Q_OBJECT
public:
    enum Stretch{Fixed=0, Small, Medium, Large, Huge};
    
    /**
     * Construct a KApplet widget just like any other widget.
     **/
    KApplet( QWidget* parent = 0, const char* name = 0 );


    /**
     * Destructor
     **/
    ~KApplet();

    /**
     * Initialize the applet according to the passed command line
     * parameters
     *
     * Evalutate some command line arguments, dock into the
     * respective applet container and eventually call
     * @ref setupGeometry().
     **/
    void init( int& argc, char ** argv );

    /**
     * Set up the applet's geometry. This function needs to be
     * reimplemented by subclasses.
     *
     * @param orientation The applet's orientation, either @p Qt::Horizontal
     * or @p Qt::Vertical.
     * @param width Width of the applet.
     * @param height Height of the applet.
     *
     * The size parameter is meant as a hint in case an applet
     * supports different look&feels depending on the target size. Of
     * course, an applet could do these things in @ref resizeEvent(), but
     * this way it's more convenient.
     *
     * The applet container that will embed this applet will resize itself
     * to the standard size of its applets. If your applet needs more
     * space, ensure to set a proper minimum size with
     * @ref QWidget::setMinimumWidth() or @ref QWidget::setMinimumHeight()
     * depending on the applets orientation.
     *
     * Keep in mind that @ref setupGeometry() may be called several times
     * during a life-cycle of your applet, for example when the applet
     * container is resized, moved or changes orientation.
     **/
    virtual void setupGeometry( Orientation orientation, int width, int height );

    /**
     * Set the applet to be fixed size or stretchable (and to what size).
     **/
    void setStretch(Stretch size);
    /**
     * @returns Integer of type @p Stretch indicating whether the applet is fixed size or stretchable (and to what size).
     **/
    Stretch stretch(){return s;}
    
    /**
     * @return A suggested size for the applet.
     **/
    QSize sizeHint() const;

    


    /**
       Returns  the current orientation set by the last KApplet::init() call.
    */
    Orientation orientation() const;



    // dcop internal
    bool process(const QCString &fun, const QByteArray &data,
		 QCString& replyType, QByteArray &replyData);

public slots:

    /*!
      Tells the applet container that the applet wants to be
      removed. Subclasses should provide a context menu with a
      "Remove" item connected to this slot.
     */
    void removeRequest();

    /*!
      Tells the applet container that the applet wants to be
      moved around. Subclasses should provide a context menu with a
      "Move" item connected to this slot.
     */
    void moveRequest();



private:
    KAppletData* d;
    Stretch s;
};


#endif
