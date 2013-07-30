TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += "/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include"
INCLUDEPATH += "/usr/include/glib-2.0"
INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"

SOURCES += \   
    main.c

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += dbus-glib-1

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += dbus-1

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gio-2.0

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gobject-2.0

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += glib-2.0



