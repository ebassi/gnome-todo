desktopdir = $(datadir)/applications
desktop_in_files = org.gnome.Todo.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

%.ini: %.ini.in
	LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
appdata_in_files = $(srcdir)/appdata/org.gnome.Todo.appdata.xml.in

@INTLTOOL_DESKTOP_RULE@

gsettings_SCHEMAS = org.gnome.todo.gschema.xml 

@GSETTINGS_RULES@

EXTRA_DIST=                     \
  appdata/org.gnome.Todo.appdata.xml \
  org.gnome.Todo.desktop \
  todo.gresource.xml \
  gtk/menus.ui \
  ui/edit-pane.ui \
  ui/initial-setup.ui \
  ui/list-view.ui \
  ui/notification.ui \
  ui/storage-dialog.ui \
  ui/storage-popover.ui \
  ui/storage-row.ui \
  ui/storage-selector.ui \
  ui/task-list-item.ui \
  ui/task-row.ui \
  ui/window.ui \
  theme/Adwaita.css \
  theme/bg.png \
  theme/bg.svg \
  $(appdata_in_files) \
  $(desktop_in_files) \
  $(gsettings_SCHEMAS)

CLEANFILES =                    \
  $(service_DATA)               \
  $(desktop_DATA)               \
  $(searchprovider_DATA)        \
  $(appdata_DATA)

-include $(top_srcdir)/git.mk
