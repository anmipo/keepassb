# Config.pri file version 2.0. Auto-generated by IDE. Any changes made by user will be lost!
BASEDIR = $$quote($$_PRO_FILE_PWD_)

device {
    CONFIG(debug, debug|release) {
        profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        } else {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }

    }

    CONFIG(release, debug|release) {
        !profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }
    }
}

simulator {
    CONFIG(debug, debug|release) {
        !profile {
            CONFIG += \
                config_pri_assets \
                config_pri_source_group1
        }
    }
}

config_pri_assets {
    OTHER_FILES += \
        $$quote($$BASEDIR/assets/ActiveFrameCover.qml) \
        $$quote($$BASEDIR/assets/LabelTextButton.qml) \
        $$quote($$BASEDIR/assets/ReadOnlyTextField.qml) \
        $$quote($$BASEDIR/assets/SearchResultsPage.qml) \
        $$quote($$BASEDIR/assets/UnlockDbPage.qml) \
        $$quote($$BASEDIR/assets/ViewEntryV4Page.qml) \
        $$quote($$BASEDIR/assets/ViewGroupPage.qml) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_empty_group.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_unlock.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/password_hidden.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/password_visible.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/0.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/1.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/10.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/11.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/12.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/13.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/14.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/15.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/16.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/17.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/18.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/19.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/2.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/20.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/21.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/22.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/23.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/24.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/25.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/26.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/27.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/28.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/29.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/3.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/30.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/31.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/32.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/33.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/34.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/35.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/36.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/37.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/38.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/39.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/4.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/40.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/41.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/42.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/43.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/44.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/45.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/46.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/47.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/48.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/49.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/5.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/50.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/51.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/52.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/53.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/54.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/55.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/56.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/57.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/58.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/59.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/6.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/60.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/61.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/62.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/63.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/64.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/65.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/66.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/67.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/68.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/7.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/8.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/pwicons/9.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_empty_group.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_unlock.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/password_hidden.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/password_visible.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/0.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/1.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/10.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/11.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/12.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/13.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/14.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/15.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/16.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/17.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/18.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/19.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/2.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/20.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/21.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/22.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/23.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/24.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/25.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/26.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/27.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/28.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/29.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/3.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/30.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/31.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/32.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/33.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/34.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/35.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/36.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/37.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/38.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/39.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/4.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/40.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/41.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/42.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/43.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/44.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/45.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/46.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/47.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/48.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/49.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/5.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/50.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/51.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/52.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/53.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/54.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/55.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/56.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/57.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/58.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/59.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/6.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/60.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/61.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/62.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/63.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/64.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/65.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/66.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/67.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/68.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/7.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/8.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/pwicons/9.png) \
        $$quote($$BASEDIR/assets/images/cover.png) \
        $$quote($$BASEDIR/assets/images/ic_attach.png) \
        $$quote($$BASEDIR/assets/images/ic_browse.png) \
        $$quote($$BASEDIR/assets/images/ic_copy.png) \
        $$quote($$BASEDIR/assets/images/ic_copy_link.png) \
        $$quote($$BASEDIR/assets/images/ic_copy_password.png) \
        $$quote($$BASEDIR/assets/images/ic_done.png) \
        $$quote($$BASEDIR/assets/images/ic_edit.png) \
        $$quote($$BASEDIR/assets/images/ic_entry.png) \
        $$quote($$BASEDIR/assets/images/ic_history.png) \
        $$quote($$BASEDIR/assets/images/ic_notes.png) \
        $$quote($$BASEDIR/assets/images/ic_rename.png) \
        $$quote($$BASEDIR/assets/images/ic_save.png) \
        $$quote($$BASEDIR/assets/images/ic_search.png) \
        $$quote($$BASEDIR/assets/images/ic_view_details.png) \
        $$quote($$BASEDIR/assets/images/warning.png) \
        $$quote($$BASEDIR/assets/main.qml)
}

config_pri_source_group1 {
    SOURCES += \
        $$quote($$BASEDIR/src/applicationui.cpp) \
        $$quote($$BASEDIR/src/crypto/CryptoManager.cpp) \
        $$quote($$BASEDIR/src/db/PwDatabase.cpp) \
        $$quote($$BASEDIR/src/db/PwEntry.cpp) \
        $$quote($$BASEDIR/src/db/PwGroup.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwDatabaseV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwEntryV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwGroupV4.cpp) \
        $$quote($$BASEDIR/src/main.cpp) \
        $$quote($$BASEDIR/src/ui/ActiveFrame.cpp) \
        $$quote($$BASEDIR/src/util/TimedClipboard.cpp)

    HEADERS += \
        $$quote($$BASEDIR/src/applicationui.hpp) \
        $$quote($$BASEDIR/src/crypto/CryptoManager.h) \
        $$quote($$BASEDIR/src/db/PwDatabase.h) \
        $$quote($$BASEDIR/src/db/PwEntry.h) \
        $$quote($$BASEDIR/src/db/PwGroup.h) \
        $$quote($$BASEDIR/src/db/v4/PwDatabaseV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwEntryV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwGroupV4.h) \
        $$quote($$BASEDIR/src/ui/ActiveFrame.h) \
        $$quote($$BASEDIR/src/util/AppConfig.h) \
        $$quote($$BASEDIR/src/util/TimedClipboard.h)
}

INCLUDEPATH += $$quote($$BASEDIR/src/db/v4) \
    $$quote($$BASEDIR/src/util) \
    $$quote($$BASEDIR/src/db) \
    $$quote($$BASEDIR/src/ui) \
    $$quote($$BASEDIR/src) \
    $$quote($$BASEDIR/src/crypto)

CONFIG += precompile_header

PRECOMPILED_HEADER = $$quote($$BASEDIR/precompiled.h)

lupdate_inclusion {
    SOURCES += \
        $$quote($$BASEDIR/../src/*.c) \
        $$quote($$BASEDIR/../src/*.c++) \
        $$quote($$BASEDIR/../src/*.cc) \
        $$quote($$BASEDIR/../src/*.cpp) \
        $$quote($$BASEDIR/../src/*.cxx) \
        $$quote($$BASEDIR/../src/crypto/*.c) \
        $$quote($$BASEDIR/../src/crypto/*.c++) \
        $$quote($$BASEDIR/../src/crypto/*.cc) \
        $$quote($$BASEDIR/../src/crypto/*.cpp) \
        $$quote($$BASEDIR/../src/crypto/*.cxx) \
        $$quote($$BASEDIR/../src/db/*.c) \
        $$quote($$BASEDIR/../src/db/*.c++) \
        $$quote($$BASEDIR/../src/db/*.cc) \
        $$quote($$BASEDIR/../src/db/*.cpp) \
        $$quote($$BASEDIR/../src/db/*.cxx) \
        $$quote($$BASEDIR/../src/db/v4/*.c) \
        $$quote($$BASEDIR/../src/db/v4/*.c++) \
        $$quote($$BASEDIR/../src/db/v4/*.cc) \
        $$quote($$BASEDIR/../src/db/v4/*.cpp) \
        $$quote($$BASEDIR/../src/db/v4/*.cxx) \
        $$quote($$BASEDIR/../src/ui/*.c) \
        $$quote($$BASEDIR/../src/ui/*.c++) \
        $$quote($$BASEDIR/../src/ui/*.cc) \
        $$quote($$BASEDIR/../src/ui/*.cpp) \
        $$quote($$BASEDIR/../src/ui/*.cxx) \
        $$quote($$BASEDIR/../src/util/*.c) \
        $$quote($$BASEDIR/../src/util/*.c++) \
        $$quote($$BASEDIR/../src/util/*.cc) \
        $$quote($$BASEDIR/../src/util/*.cpp) \
        $$quote($$BASEDIR/../src/util/*.cxx) \
        $$quote($$BASEDIR/../assets/*.qml) \
        $$quote($$BASEDIR/../assets/*.js) \
        $$quote($$BASEDIR/../assets/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/images/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/images/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/images/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/pwicons/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/pwicons/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.bright/pwicons/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/images/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/images/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/images/*.qs) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/pwicons/*.qml) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/pwicons/*.js) \
        $$quote($$BASEDIR/../assets/VisualStyle.dark/pwicons/*.qs) \
        $$quote($$BASEDIR/../assets/images/*.qml) \
        $$quote($$BASEDIR/../assets/images/*.js) \
        $$quote($$BASEDIR/../assets/images/*.qs)

    HEADERS += \
        $$quote($$BASEDIR/../src/*.h) \
        $$quote($$BASEDIR/../src/*.h++) \
        $$quote($$BASEDIR/../src/*.hh) \
        $$quote($$BASEDIR/../src/*.hpp) \
        $$quote($$BASEDIR/../src/*.hxx)
}

TRANSLATIONS = $$quote($${TARGET}.ts)
