install( FILES
 kservice.h
 kservicetype.h
 kservicetypeprofile.h
 kservicetypetrader.h
 kserviceoffer.h
 ktrader.h
 kplugininfo.h
 DESTINATION ${INCLUDE_INSTALL_DIR})
install( FILES kplugininfo.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

# not installed: kservicefactory.h kservicetypefactory.h
