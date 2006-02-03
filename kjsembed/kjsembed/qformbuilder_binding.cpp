#include <QWidget>
#include <QtDesigner/QFormBuilder>
#include <QFile>
#include <QDebug>

#include <kjs/object.h>

#include "qformbuilder_binding.h"
#include "static_binding.h"
#include "global.h"

using namespace KJSEmbed;
NO_METHODS( FormBuilder )
NO_ENUMS( FormBuilder )
NO_STATICS( FormBuilder )

START_CTOR( FormBuilder, Form, 2 )
    if( args.size() > 0 )
    {
        QFormBuilder *formBuilder = new QFormBuilder();
        QWidget *parentWidget = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentWidget = parentImp->object<QWidget>();
        }
        QString fileName = args[0]->toString(exec).qstring();
        QFile uiFile(fileName);
        if( uiFile.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QWidget *returnWidget = formBuilder->load(&uiFile,parentWidget);
            uiFile.close();
            if( returnWidget == 0 )
            {
                delete formBuilder;
                return KJS::throwError(exec, KJS::GeneralError, i18n("There was an error reading the file '%1'").arg(fileName));
                //return KJSEmbed::throwError(exec, i18n("There was an error reading the file '%1'").arg(fileName));
            }
            KJS::JSObject *form = new KJSEmbed::QObjectBinding( exec, returnWidget );
            delete formBuilder;
            return form;
        }
                // Throw error could not read file
        delete formBuilder;
        return KJS::throwError(exec, KJS::GeneralError, i18n("Could not read file '%1'").arg(fileName));
        // return KJSEmbed::throwError(exec, i18n("Could not read file '%1'").arg(fileName));
    }
            // Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a filename."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a filename."));
END_CTOR
