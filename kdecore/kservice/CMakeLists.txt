install_files( /include FILES
 kservice.h
 kservicetype.h
 kservicetypeprofile.h
 kservicetypetrader.h
 kserviceoffer.h
 ktrader.h
 kplugininfo.h
)
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kplugininfo.desktop )

# not installed: kservicefactory.h kservicetypefactory.h
