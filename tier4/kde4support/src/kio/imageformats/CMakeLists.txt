# Image format plugins supplied with Qt
install( FILES
  png.desktop
  xpm.desktop
  bmp.desktop
  pbm.desktop
  pgm.desktop
  ppm.desktop
  xbm.desktop
  jpeg.desktop
  ico.desktop
  gif.desktop
  mng.desktop
  tiff.desktop
  svg+xml.desktop
  svg+xml-compressed.desktop
  wbmp.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/qimageioplugins/ )

install( FILES qimageio_plugin.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR}/ )
