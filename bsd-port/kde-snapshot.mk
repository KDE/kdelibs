#$Id$

CONFIGURE_ENV=	CXXFLAGS="$(CFLAGS)" \
		install_root=$(INSTALL_ROOT) \
		INSTALL_SCRIPT="$(INSTALL_SCRIPT)"

# Since there's nothing to fetch, we might as well use a dummy target
do-fetch:
		@true
# This should clean the KDE target pretty well
pre-clean:
		cd $(WRKSRC);$(GMAKE) clean   

# We need to go through Makefile.cvs before anything else.
pre-configure:
		cd $(WRKSRC);rm -f config.cache;$(GMAKE) -f Makefile.cvs
		rm -f $(PLIST)
post-install:
		${MAKE} PREFIX=${PREFIX} make-plist
		cp -Rp ${INSTALL_ROOT}/* /
		rm -rf ${INSTALL_ROOT}
		ldconfig -m ${PREFIX}/lib

# This should finally work somewhat decently now
make-plist:
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find . -type f ! -name .|sed 's,^\./,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find . -type l ! -name .|sed 's,^\./,,'|sort >> $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find . -type d ! -name .|sed 's,^\./,@dirrm ,'|sort -r>> $(PLIST)
		@echo "@exec /sbin/ldconfig -R" >> $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)
