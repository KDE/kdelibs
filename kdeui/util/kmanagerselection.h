/****************************************************************************

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef KMANAGERSELECTION_H
#define KMANAGERSELECTION_H

#include <kdeui_export.h>
#include <QtCore/QObject>

#ifdef Q_WS_X11 // FIXME(E)

#include <X11/Xlib.h>
#include <fixx11h.h>


/**
 This class implements claiming and owning manager selections, as described
 in the ICCCM, section 2.8. The selection atom is passed to the constructor,
 claim() attemps to claim ownership of the selection, release() gives up
 the selection ownership. Signal lostOwnership() is emitted when the selection
 is claimed by another owner.
 @short ICCCM manager selection owner
*/
class KDEUI_EXPORT KSelectionOwner
    : public QObject
    {
    Q_OBJECT
    public:
        /**
         * This constructor initializes the object, but doesn't perform any
         * operation on the selection.
         *
         * @param selection atom representing the manager selection
         * @param screen X screen, or -1 for default
         * @param parent parent object, or NULL if there is none
         */
        explicit KSelectionOwner( Atom selection, int screen = -1, QObject* parent = NULL );
        /**
         * @overload
         * This constructor accepts the selection name and creates the appropriate atom
         * for it automatically.
         *
         * @param selection name of the manager selection
         * @param screen X screen, or -1 for default
         * @param parent parent object, or NULL if there is none
         */
        explicit KSelectionOwner( const char* selection, int screen = -1, QObject* parent = NULL );
        /**
         * Destructor. Calls release().
         */
	virtual ~KSelectionOwner();
        /**
         * This function attemps to claim ownership of the manager selection, using
         * the current X timestamp. If @p force is false, and the selection is already
         * owned, the selection is not claimed, and false is returned. If claiming
         * is forced and the selection is owned by another client, it is waited for up to 1 second
         * for the previous owner to disown the selection, if @p force_kill is true,
         * and the previous owner fails to disown the selection in time,
         * it will be forcibly killed. True is returned after successfully claiming
         * ownership of the selection.
         */
        bool claim( bool force, bool force_kill = true );
        /**
         * If the selection is owned, the ownership is given up.
         */
        void release();
        /**
         * If the selection is owned, returns the window used internally
         * for owning the selection.
         */
        Window ownerWindow() const; // None if not owning the selection
        /**
         * @internal
         */
	bool filterEvent( XEvent* ev_P ); // internal
    Q_SIGNALS:
        /**
         * This signal is emitted if the selection was owned and the ownership
         * has been lost due to another client claiming it, this signal is emitted.
         * IMPORTANT: It's not safe to delete the instance in a slot connected
         * to this signal.
         */
        void lostOwnership();
    protected:
        /**
         * Called for every X event received on the window used for owning
         * the selection. If true is returned, the event is filtered out.
         */
        virtual bool handleMessage( XEvent* ev );
        /**
         * Called when a SelectionRequest event is received. A reply should
         * be sent using the selection handling mechanism described in the ICCCM
         * section 2.
         *
         * @param target requested target type
         * @param property property to use for the reply data
         * @param requestor requestor window
         */
        virtual bool genericReply( Atom target, Atom property, Window requestor );
        /**
         * Called to announce the supported targets, as described in the ICCCM
         * section 2.6. The default implementation announces the required targets
         * MULTIPLE, TIMESTAMP and TARGETS.
         */
        virtual void replyTargets( Atom property, Window requestor );
        /**
         * Called to create atoms needed for claiming the selection and
         * communication using the selection handling mechanism. The default
         * implementation must be called if reimplemented. This method
         * may be called repeatedly.
         */
        virtual void getAtoms();
        /**
         * Sets extra data to be sent in the message sent to root window
         * after successfully claiming a selection. These extra data
         * are in data.l[3] and data.l[4] fields of the XClientMessage.
         */
        void setData( long extra1, long extra2 );
    private:
        void filter_selection_request( XSelectionRequestEvent& ev_P );
        bool handle_selection( Atom target_P, Atom property_P, Window requestor_P );

        class Private;
        Private* const d;
    };

/**
 This class implements watching manager selections, as described in the ICCCM
 section 2.8. It emits signal newOwner() when a new owner claim the selection,
 and emits lostOwner() when the selection ownership is given up. To find
 out current owner of the selection, owner() can be used.
 @short ICCCM manager selection watching
*/
class KDEUI_EXPORT KSelectionWatcher
    : public QObject
    {
    Q_OBJECT
    public:
        /**
         * This constructor initializes the object, but doesn't perform any
         * operation on the selection.
         *
         * @param selection atom representing the manager selection
         * @param screen X screen, or -1 for default
         * @param parent parent object, or NULL if there is none
         */
        explicit KSelectionWatcher( Atom selection, int screen = -1, QObject* parent = NULL );
        /**
         * @overload
         * This constructor accepts the selection name and creates the appropriate atom
         * for it automatically.
         *
         * @param selection name of the manager selection
         * @param screen X screen, or -1 for default
         * @param parent parent object, or NULL if there is none
         */
        explicit KSelectionWatcher( const char* selection, int screen = -1, QObject* parent = NULL );
	virtual ~KSelectionWatcher();
        /**
         * Return the current owner of the manager selection, if any. Note that if the event
         * informing about the owner change is still in the input queue, newOwner() might
         * have been emitted yet.
         */
        Window owner();
        /**
         * @internal
         */
        void filterEvent( XEvent* ev_P ); // internal
    Q_SIGNALS:
        /**
         * This signal is emitted when the selection is successfully claimed by a new
         * owner.
         * @param owner the new owner of the selection
         */
        void newOwner( Window owner );
        /**
         * This signal is emitted when the selection is given up, i.e. there's no
         * owner. Note that the selection may be immediatelly claimed again,
         * so the newOwner() signal may be emitted right after this one.
         * It's safe to delete the instance in a slot connected to this signal.
         */
        void lostOwner();
    private:
        void init();

        class Private;
        Private* const d;
    };

#endif
#endif
