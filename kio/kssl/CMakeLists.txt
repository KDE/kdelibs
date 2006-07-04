
add_subdirectory( kssl )

########### install files ###############
install( FILES kssl.h                        ksslcertificatehome.h         ksslsettings.h                ksslcertificate.h             ksslconnectioninfo.h          ksslcertificatefactory.h      ksslcertificatecache.h        ksslpeerinfo.h                ksslinfodlg.h                 ksslcertdlg.h                 ksslutils.h                   kopenssl.h                    ksslall.h            ksslpkcs12.h            ksslcertchain.h            ksslkeygen.h            ksslx509v3.h            ksslsigners.h           ksslpkcs7.h          ksslpemcallback.h       ksslconfig.h            ksslsession.h           ksslx509map.h           ksmimecrypto.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )





