set(phonon_fake_PART_SRCS
   abstractaudiooutput.cpp
   audiodataoutput.cpp
   delayaudioeffect.cpp
   effect.cpp
   audiooutput.cpp
   mediaobject.cpp
   stream.cpp
   videodataoutput.cpp
   visualization.cpp
   backend.cpp
   volumefadereffect.cpp
   videowidget.cpp
   )

kde4_add_library(phonon_fake SHARED ${phonon_fake_PART_SRCS})
target_link_libraries(phonon_fake ${KDE4_KDECORE_LIBS} ${KDE4_PHONON_LIBS} phononexperimental)
