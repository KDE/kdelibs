#include <QDebug>
#include <QUrl>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>

#include <kjs/object.h>

#include "bind_qtextedit.h"

using namespace KJSEmbed;

namespace TextEditNS {

START_QOBJECT_METHOD( alignment, QTextEdit )
    int i = (int) object->alignment();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( append, QTextEdit )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    object->append( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( anchorAt, QTextEdit )
    QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
    QString anchor = object->anchorAt( arg0 );
    result = KJSEmbed::createValue( exec, "String", anchor );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( currentFont, QTextEdit )
    QFont font = object->currentFont();
    result = KJSEmbed::createValue( exec, "QFont", font );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( cursorRect, QTextEdit )
    // Handle the case where we get an argument
    // QRect cursorRect ( const QTextCursor & cursor ) const
    QRect rect = object->cursorRect();
    result = KJSEmbed::createValue( exec, "QRect", rect );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( ensureCursorVisible, QTextEdit )
    object->ensureCursorVisible();
END_QOBJECT_METHOD

START_QOBJECT_METHOD( find, QTextEdit )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1, 0);
    bool b = object->find( arg0, (QTextDocument::FindFlags) arg1 );
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( fontFamily, QTextEdit )
    QString s = object->fontFamily();
    result = KJS::String( s );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( fontItalic, QTextEdit )
    bool b = object->fontItalic();
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( fontPointSize, QTextEdit )
    double d = object->fontPointSize();
    result = KJS::Number( d );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( fontUnderline, QTextEdit )
    bool b = object->fontUnderline();
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( fontWeight, QTextEdit )
    int i = object->fontWeight();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( isReadOnly, QTextEdit )
    bool b= object->isReadOnly();
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( isUndoRedoEnabled, QTextEdit )
    bool b = object->isUndoRedoEnabled();
    result = KJS::Boolean( b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( lineWrapColumnOrWidth, QTextEdit )
    int i = object->lineWrapColumnOrWidth();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( lineWrapMode, QTextEdit )
    int i = object->lineWrapMode();
    result = KJS::Number( i );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( loadResource, QTextEdit )
    int arg0 = KJSEmbed::extractInt( exec, args, 0 );
    QUrl arg1 = KJSEmbed::extractValue<QUrl>( exec, args, 1 );
    QVariant v = object->loadResource( arg0, arg1 );
    result = convertToValue( exec, v );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setReadOnly, QTextEdit )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    object->setReadOnly( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setDocumentTitle, QTextEdit )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    object->setDocumentTitle( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setLineWrapColumnOrWidth, QTextEdit )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    object->setLineWrapColumnOrWidth( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setLineWrapMode, QTextEdit )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    object->setLineWrapMode( (QTextEdit::LineWrapMode) arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setTabChangesFocus, QTextEdit )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    object->setTabChangesFocus( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( setUndoRedoEnabled, QTextEdit )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    object->setUndoRedoEnabled( arg0 );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( tabChangesFocus, QTextEdit )
    bool b= object->tabChangesFocus();
    result = KJSEmbed::createValue( exec, "Bool", b );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( textColor, QTextEdit )
    QColor c = object->textColor();
    qDebug() << "color is " << c.name() << endl;
    result = KJSEmbed::createValue( exec, "QColor", c );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( toHtml, QTextEdit )
    QString s = object->toHtml();
    result = KJS::String( s );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( toPlainText, QTextEdit )
    QString s = object->toPlainText();
    result = KJS::String( s );
END_QOBJECT_METHOD

};

NO_ENUMS( TextEdit )

START_METHOD_LUT( TextEdit )
        {"alignment", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::alignment },
        {"append", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::append },
        {"anchorAt", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::anchorAt },
        {"currentFont", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::currentFont },
        {"cursorRect", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::cursorRect },
        {"ensureCursorVisible", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::ensureCursorVisible },
        {"find", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::find },
        {"fontFamily", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::fontFamily },
        {"fontItalic", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::fontItalic },
        {"fontPointSize", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::fontPointSize },
        {"fontUnderline", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::fontUnderline },
        {"fontWeight", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::fontWeight },
        {"isReadOnly", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::isReadOnly },
        {"isUndoRedoEnabled", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::isUndoRedoEnabled },
        {"lineWrapColumnOrWidth", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::lineWrapColumnOrWidth },
        {"lineWrapMode", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::lineWrapMode },
        {"loadResource", 2, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::loadResource },
        {"setDocumentTitle", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setDocumentTitle },
        {"setLineWrapColumnOrWidth", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setLineWrapColumnOrWidth },
        {"setLineWrapMode", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setLineWrapMode },
        {"setReadOnly", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setReadOnly },
        {"setTabChangesFocus", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setTabChangesFocus },
        {"setUndoRedoEnabled", 1, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::setUndoRedoEnabled },
        {"textColor", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::textColor },
        {"toHtml", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::toHtml },
        {"toPlainText", 0, KJS::DontDelete|KJS::ReadOnly, &TextEditNS::toPlainText }
END_METHOD_LUT

NO_STATICS( TextEdit )


/*
 * Constructor
 * arg1 Name of the binding
 * arg2 Name of the class in JS
 * arg3 minimum number of args
 */
START_CTOR( TextEdit, QTextEdit, 0 )
   QTextEdit *te = new QTextEdit();
   KJSEmbed::QObjectBinding *obj = new KJSEmbed::QObjectBinding( exec, te );

   StaticBinding::publish( exec, obj, TextEdit::methods() );

   return obj;
END_CTOR


