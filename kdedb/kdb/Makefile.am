headers_kdbcore = $(shell cd $(top_srcdir)/kdedb/kdbcore && ls *.h)
headers_kdbui = $(shell cd $(top_srcdir)/kdedb/kdbui && ls *.h)

all-am:
	@for file in $(headers_kdbcore) ; \
	do \
		if [ ! -f $$file ]; then \
			$(LN_S) $(top_srcdir)/kdedb/kdbcore/$$file $$file ; \
		fi \
	done

	@for file in $(headers_kdbui) ; \
	do \
		if [ ! -f $$file ]; then \
			$(LN_S) $(top_srcdir)/kdedb/kdbui/$$file $$file ; \
		fi \
	done

clean-am:
	rm -f $(headers_kdbcore) $(headers_kdbui)
