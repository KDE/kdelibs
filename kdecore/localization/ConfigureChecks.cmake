## Should this be find_package(Gettext) instead, which seems more complete?
find_package(Libintl)
macro_log_feature(LIBINTL_FOUND "Libintl" "Support for multiple languages" "http://www.gnu.org/software/gettext" TRUE "" "Enables KDE to be available in many different languages")
