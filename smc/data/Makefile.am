install-data-local: uninstall-local
	mkdir -p "$(DESTDIR)$(pkgdatadir)"; \
	for dir in * ; do \
		test -d "$$dir" || continue; \
		cp -r "$$dir" "$(DESTDIR)$(pkgdatadir)"; \
	done

uninstall-local:
	-test -d "$(DESTDIR)$(pkgdatadir)" && rm -rf "$(DESTDIR)$(pkgdatadir)"
