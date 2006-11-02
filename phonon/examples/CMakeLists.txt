MACRO( PHONON_DECLARE_EXAMPLES )
	FOREACH( _examplename ${ARGN} )
		kde4_automoc( ${_examplename}.cpp )
		kde4_add_executable( ${_examplename} ${_examplename}.cpp )
		target_link_libraries( ${_examplename} ${KDE4_KIO_LIBS} phononcore phononui )
	ENDFOREACH( _examplename )
ENDMACRO( PHONON_DECLARE_EXAMPLES )

PHONON_DECLARE_EXAMPLES(crossfade videoplayandforget pcmoutput)
