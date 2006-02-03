#include "url.h"

#include <QDebug>
#include <QUrl>

using namespace KJSEmbed;

UrlBinding::UrlBinding( KJS::ExecState *exec, const QUrl &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, ValueFactory::methods() );
    StaticBinding::publish( exec, this, Url::methods() );
}

namespace UrlNS
{

START_VARIANT_METHOD( callisValid, QUrl )
    bool cppValue = value.isValid();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( toString, QUrl )
    QUrl::FormattingOptions opts = (QUrl::FormattingOptions)KJSEmbed::extractInt(exec, args, 0, QUrl::None);
    result = KJS::String( value.toString( opts ) );
END_VARIANT_METHOD
}

START_METHOD_LUT( Url )
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &UrlNS::toString},
    {"isValid", 0, KJS::DontDelete|KJS::ReadOnly, &UrlNS::callisValid}
END_METHOD_LUT

START_ENUM_LUT( Url )
    {"None", QUrl::None},
    {"RemoveScheme", QUrl::RemoveScheme},
    {"RemovePassword", QUrl::RemovePassword},
    {"RemoveUserInfo", QUrl::RemoveUserInfo},
    {"RemovePort", QUrl::RemovePort},
    {"RemoveAuthority", QUrl::RemoveAuthority},
    {"RemovePath", QUrl::RemovePath},
    {"RemoveQuery", QUrl::RemoveQuery},
    {"RemoveFragment", QUrl::RemoveFragment},
    {"StripTrailingSlash", QUrl::StripTrailingSlash}
END_ENUM_LUT

NO_STATICS( Url )

START_CTOR( Url, QUrl, 0 )
    if( args.size() == 1 )
    {
        return new KJSEmbed::UrlBinding(exec, QUrl( KJSEmbed::extractQString( exec, args, 0 ) ) );
    }

    return new KJSEmbed::UrlBinding( exec, QUrl() );
END_CTOR

