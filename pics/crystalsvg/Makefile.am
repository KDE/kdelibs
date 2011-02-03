KDE_ICON=AUTO

crystaldir=$(kde_icondir)/crystalsvg
crystal_DATA=index.theme

generate:
	@echo "Converting SVG files to PNG files, starting..."
	@echo "Sizes: 16x16; 22x22; 32x32; 48x48; 64x64; 128x128"
	@rm -f $(top_builddir)/pics/crystalsvg/generated -Rf
	@mkdir $(top_builddir)/pics/crystalsvg/generated

	@for file in $(top_srcdir)/pics/crystalsvg/*.svg*; do \
		echo "Processing: $$file..."; \
		$(top_builddir)/pics/ksvgtopng 16 16 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr16/' | sed -e 's/svg\|svgz/png/'`; \
		$(top_builddir)/pics/ksvgtopng 22 22 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr22/' | sed -e 's/svg\|svgz/png/'`; \
		$(top_builddir)/pics/ksvgtopng 32 32 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr32/' | sed -e 's/svg\|svgz/png/'`; \
		$(top_builddir)/pics/ksvgtopng 48 48 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr48/' | sed -e 's/svg\|svgz/png/'`; \
		$(top_builddir)/pics/ksvgtopng 64 64 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr64/' | sed -e 's/svg\|svgz/png/'`; \
		$(top_builddir)/pics/ksvgtopng 128 128 $$file $(top_builddir)/pics/crystalsvg/generated/`basename $$file | sed -e 's/crsc/cr128/' | sed -e 's/svg\|svgz/png/'`; \
	done

.PHONY: generate

