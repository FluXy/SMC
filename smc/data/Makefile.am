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
	done ); \
	find "$(DESTDIR)$(pkgdatadir)" \
		-type d -exec chmod 755 {} \; -o \
		-type f -exec chmod 644 {} \;

uninstall-local:
	rm -rf "$(DESTDIR)$(pkgdatadir)"
