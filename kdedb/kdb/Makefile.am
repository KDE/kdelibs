headers_kdbcore = $(shell cd $(top_srcdir)/kdedb/kdbcore && ls *.h)
headers_kdbui = $(shell cd $(top_srcdir)/kdedb/kdbui && ls *.h)

all-am:
	@for file in $(headers_kdbcore) ; \
	do \
		ln -sf $(top_srcdir)/kdedb/kdbcore/$$file $$file ; \
	done

	@for file in $(headers_kdbui) ; \
	do \
		ln -sf $(top_srcdir)/kdedb/kdbui/$$file $$file ; \
	done

clean-am:
	rm -f $(headers_kdbcore) $(headers_kdbui)
