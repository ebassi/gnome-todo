AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DUI_DATA_DIR=\""$(pkgdatadir)/style"\" \
	-DGOA_API_IS_SUBJECT_TO_CHANGE \
	-I$(srcdir)/storage \
	-I$(srcdir)/notification

bin_PROGRAMS = gnome-todo

BUILT_SOURCES = \
	gtd-resources.c \
	gtd-resources.h

gnome_todo_SOURCES = \
	$(BUILT_SOURCES) \
	notification/gtd-notification.c \
	notification/gtd-notification.h \
	notification/gtd-notification-widget.c \
	notification/gtd-notification-widget.h \
	storage/gtd-storage.c \
	storage/gtd-storage.h \
	storage/gtd-storage-dialog.c \
	storage/gtd-storage-dialog.h \
	storage/gtd-storage-popover.c \
	storage/gtd-storage-popover.h \
	storage/gtd-storage-row.c \
	storage/gtd-storage-row.h \
	storage/gtd-storage-selector.c \
	storage/gtd-storage-selector.h \
	gtd-application.c \
	gtd-application.h \
	gtd-arrow-frame.c \
	gtd-arrow-frame.h \
	gtd-edit-pane.c \
	gtd-edit-pane.h \
	gtd-enums.h \
	gtd-initial-setup-window.c \
	gtd-initial-setup-window.h \
	gtd-manager.c \
	gtd-manager.h \
	gtd-object.c \
	gtd-object.h \
	gtd-task.c \
	gtd-task.h \
	gtd-task-list.c \
	gtd-task-list.h \
	gtd-task-list-item.c \
	gtd-task-list-item.h \
	gtd-task-list-view.c \
	gtd-task-list-view.h \
	gtd-task-row.c \
	gtd-task-row.h \
	gtd-types.h \
	gtd-window.c \
	gtd-window.h \
	main.c

gnome_todo_CFLAGS = \
	$(GNOME_TODO_CFLAGS) \
	$(GNOME_TODO_WARN_CFLAGS)

gnome_todo_LDFLAGS = \
	-Wl,--export-dynamic \
	$(GNOME_TODO_WARN_LDFLAGS)

gnome_todo_LDADD = \
	$(GNOME_TODO_LIBS)

resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/todo.gresource.xml)
gtd-resources.c: $(top_srcdir)/data/todo.gresource.xml $(resource_files)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(top_srcdir)/data --generate-source --c-name todo $(top_srcdir)/data/todo.gresource.xml
gtd-resources.h: $(top_srcdir)/data/todo.gresource.xml $(resource_files)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(top_srcdir)/data --generate-header --c-name todo $(top_srcdir)/data/todo.gresource.xml

CLEANFILES = \
	$(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
