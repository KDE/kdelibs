#$Id$

OSVERSION!=     sysctl -n kern.osreldate

CONFIGURE_ENV=	CXXFLAGS="$(CFLAGS)" \
		install_root=$(INSTALL_ROOT) \
		INSTALL_SCRIPT="install -c -m 555"

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

# This should finally work somewhat decently now
make-plist:
.if ${OSVERSION} >= 300000
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type f|sed 's,^\./,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type l|sed 's,^\./,,'|sort >> $(PLIST)
		@echo "@exec /sbin/ldconfig -m %B" >> $(PLIST)
		@echo "@exec /sbin/ldconfig -R" >> $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type d|sed 's,^\./,@dirrm ,'|sort -r>> $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)
.else
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type f|sed 's,^\.//,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type l|sed 's,^\.//,,'|sort >> $(PLIST)
		@echo "@exec /sbin/ldconfig -m %B" >> $(PLIST)
		@echo "@exec /sbin/ldconfig -R" >> $(PLIST)
		cd ${INSTALL_ROOT}/${PREFIX} && \
			find ./ -type d|sed 's,^\.//,@dirrm ,'|sort -r>> $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)
.endif
