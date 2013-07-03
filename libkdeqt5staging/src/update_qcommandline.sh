cp -f /d/qt/5/qtbase-cli_parser_rebased/src/corelib/tools/qcommand* .
perl -pi -e 's/Q_CORE_EXPORT/KDEQT5STAGING_EXPORT/g' qc*.h
perl -pi -e 's/QtCore\///' qc*.h
perl -pi -e '$_ = "#include \"kdeqt5staging_export.h\"\n" . $_ if (/qstringlist.h/)' qcommandlineoption.h
