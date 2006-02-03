#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjs/object.h>

#include "bind_qtextcursor.h"

using namespace KJSEmbed;

namespace TextCursorNS {

START_OBJECT_METHOD( anchor, QTextCursor )
    int i = object->anchor();
    result = KJS::Number( i );
END_OBJECT_METHOD

START_OBJECT_METHOD( atBlockEnd, QTextCursor )
    bool b = object->atBlockEnd();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( atBlockStart, QTextCursor )
    bool b = object->atBlockStart();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( atEnd, QTextCursor )
    bool b = object->atEnd();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( atStart, QTextCursor )
    bool b = object->atStart();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( beginEditBlock, QTextCursor )
    object->beginEditBlock();
END_OBJECT_METHOD

START_OBJECT_METHOD( clearSelection, QTextCursor )
    object->clearSelection();
END_OBJECT_METHOD

START_OBJECT_METHOD( deleteChar, QTextCursor )
    object->deleteChar();
END_OBJECT_METHOD

START_OBJECT_METHOD( deletePreviousChar, QTextCursor )
    object->deletePreviousChar();
END_OBJECT_METHOD

START_OBJECT_METHOD( endEditBlock, QTextCursor )
    object->endEditBlock();
END_OBJECT_METHOD

START_OBJECT_METHOD( hasComplexSelection, QTextCursor )
    bool b = object->hasComplexSelection();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( hasSelection, QTextCursor )
    bool b = object->hasSelection();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( insertBlock, QTextCursor )
    object->insertBlock();
END_OBJECT_METHOD

START_OBJECT_METHOD( insertImage, QTextCursor )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    object->insertImage( arg0 );
END_OBJECT_METHOD

START_OBJECT_METHOD( insertText, QTextCursor )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    object->insertText( arg0 );
END_OBJECT_METHOD

START_OBJECT_METHOD( isNull, QTextCursor )
    bool b = object->isNull();
    result = KJS::Boolean( b );
END_OBJECT_METHOD

START_OBJECT_METHOD( joinPreviousEditBlock, QTextCursor )
    object->joinPreviousEditBlock();
END_OBJECT_METHOD

START_OBJECT_METHOD( position, QTextCursor )
    int i = object->position();
    result = KJS::Number( i );
END_OBJECT_METHOD

START_OBJECT_METHOD( removeSelectedText, QTextCursor )
    object->removeSelectedText();
END_OBJECT_METHOD

START_OBJECT_METHOD( select, QTextCursor )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    object->select( (QTextCursor::SelectionType) arg0 );
END_OBJECT_METHOD

START_OBJECT_METHOD( selectedText, QTextCursor )
    QString s = object->selectedText();
    result = KJS::String( s );
END_OBJECT_METHOD

START_OBJECT_METHOD( selectionStart, QTextCursor )
    int i = object->selectionStart();
    result = KJS::Number( i );
END_OBJECT_METHOD

START_OBJECT_METHOD( selectionEnd, QTextCursor )
    int i = object->selectionEnd();
    result = KJS::Number( i );
END_OBJECT_METHOD

};

NO_ENUMS( TextCursor )

START_METHOD_LUT( TextCursor )
        {"anchor", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::anchor },
        {"atBlockEnd", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::atBlockEnd },
        {"atBlockStart", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::atBlockStart },
        {"atEnd", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::atEnd },
        {"atStart", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::atStart },
        {"beginEditBlock", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::beginEditBlock },
        {"clearSelection", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::clearSelection },
        {"deleteChar", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::deleteChar },
        {"deletePreviousChar", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::deletePreviousChar },
        {"endEditBlock", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::endEditBlock },
        {"hasComplexSelection", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::hasComplexSelection },
        {"hasSelection", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::hasSelection },
        {"insertBlock", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::insertBlock },
        {"insertImage", 1, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::insertImage },
        {"insertText", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::insertText },
        {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::isNull },
        {"joinPreviousEditBlock", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::joinPreviousEditBlock },
        {"position", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::position },
        {"removeSelectedText", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::removeSelectedText },
        {"select", 1, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::select },
        {"selectedText", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::selectedText },
        {"selectionStart", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::selectionEnd },
        {"selectionEnd", 0, KJS::DontDelete|KJS::ReadOnly, &TextCursorNS::selectionStart }
END_METHOD_LUT

NO_STATICS( TextCursor )

START_CTOR( TextCursor, QTextCursor, 0 )
    QTextCursor *a = new QTextCursor();
    return new KJSEmbed::ObjectBinding( exec, "TextCursor", new QTextCursor() );
END_CTOR
