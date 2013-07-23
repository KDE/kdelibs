## Should this be find_package(Gettext) instead, which seems more complete?
find_package(Libintl)
set_package_properties(LIBINTL PROPERTIES DESCRIPTION "Libintl" URL "http://www.gnu.org/software/gettext"
                       TYPE REQUIRED PURPOSE "Enables KDE to be available in many different languages")
