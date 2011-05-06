EXTRA_DIST = \
	campaign \
	editor \
	gui \
	icon \
	levels \
	music \
	pixmaps \
	schema \
	sounds \
	translations \
	world

install-data-local: uninstall-local
	$(MKDIR_P) "$(DESTDIR)$(pkgdatadir)"; \
	( for d in $(EXTRA_DIST); \
		do cp -r "$(srcdir)/$$d" "$(DESTDIR)$(pkgdatadir)/"; \
	done )

uninstall-local:
	rm -rf "$(DESTDIR)$(pkgdatadir)"
