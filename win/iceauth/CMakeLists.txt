

# this can't stay here in win/, because it links to DCOP, which is not part of win/, Alex
if (win_iceauth_has_been_moved_to_kdelibs_dcop)

   add_executable(iceauth iceauth.c process.c )
   target_link_libraries(iceauth kdewin32 DCOP)

endif (win_iceauth_has_been_moved_to_kdelibs_dcop)