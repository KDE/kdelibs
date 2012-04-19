OTHER_FILES += \
               $$PWD/mimetypes.qdocconf \
               $$PWD/mimetypes-dita.qdocconf

docs_target.target = docs
docs_target.commands = qdoc3 $$PWD/mimetypes.qdocconf

ditadocs_target.target = ditadocs
ditadocs_target.commands = qdoc3 $$PWD/mimetypes-dita.qdocconf

QMAKE_EXTRA_TARGETS = docs_target ditadocs_target
QMAKE_CLEAN += \
               "-r $$PWD/html" \
               "-r $$PWD/ditaxml"

