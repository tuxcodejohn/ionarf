SUBDIRS = src


README: README.md
	ln -s README.md README

dist-hook: ChangeLog

ChangeLog: configure.ac
	git log --stat --name-only --date=short --abbrev-commit > ChangeLog

