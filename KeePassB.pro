APP_NAME = KeePassB

CONFIG += qt warn_on cascades10

LIBS += -lbbsystem -lhuapi -lbbcascadespickers -lbbdevice -lbb

CODECFORSRC = UTF-8

RESOURCES += assets.qrc
DEPENDPATH += assets

device {
	CONFIG(release, debug|release) {
		DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
	}
}

include(config.pri)
