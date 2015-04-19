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
        $$quote($$BASEDIR/assets/AboutPage.qml) \
        $$quote($$BASEDIR/assets/ActiveFrameCover.qml) \
        $$quote($$BASEDIR/assets/AppSettings.qml) \
        $$quote($$BASEDIR/assets/ChangeMasterKeyPage.qml) \
        $$quote($$BASEDIR/assets/EditEntryPage.qml) \
        $$quote($$BASEDIR/assets/EditFieldPage.qml) \
        $$quote($$BASEDIR/assets/EditGroupPage.qml) \
        $$quote($$BASEDIR/assets/GroupListItem.qml) \
        $$quote($$BASEDIR/assets/IconPickerPage.qml) \
        $$quote($$BASEDIR/assets/LabelTextButton.qml) \
        $$quote($$BASEDIR/assets/MonoTextField.qml) \
        $$quote($$BASEDIR/assets/PasswordGeneratorPage.qml) \
        $$quote($$BASEDIR/assets/QuickUnlockHelp.qml) \
        $$quote($$BASEDIR/assets/QuickUnlockPage.qml) \
        $$quote($$BASEDIR/assets/ReadOnlyTextField.qml) \
        $$quote($$BASEDIR/assets/SearchResultsPage.qml) \
        $$quote($$BASEDIR/assets/UnlockDbPage.qml) \
        $$quote($$BASEDIR/assets/ViewEntryExtrasTab.qml) \
        $$quote($$BASEDIR/assets/ViewEntryGeneralTab.qml) \
        $$quote($$BASEDIR/assets/ViewEntryHistoryTab.qml) \
        $$quote($$BASEDIR/assets/ViewEntryPage.qml) \
        $$quote($$BASEDIR/assets/ViewGroupPage.qml) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_browse.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_copy.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_copy_password.png) \
        $$quote($$BASEDIR/assets/VisualStyle.bright/images/ic_empty_group.png) \
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
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_browse.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_copy.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_copy_password.png) \
        $$quote($$BASEDIR/assets/VisualStyle.dark/images/ic_empty_group.png) \
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
        $$quote($$BASEDIR/assets/common.js) \
        $$quote($$BASEDIR/assets/demo.kdbx) \
        $$quote($$BASEDIR/assets/icons_data_model.xml) \
        $$quote($$BASEDIR/assets/images/cover-locked.png) \
        $$quote($$BASEDIR/assets/images/cover-unlocked.png) \
        $$quote($$BASEDIR/assets/images/cover.png) \
        $$quote($$BASEDIR/assets/images/ic_about.png) \
        $$quote($$BASEDIR/assets/images/ic_add_attachment.png) \
        $$quote($$BASEDIR/assets/images/ic_add_entry.png) \
        $$quote($$BASEDIR/assets/images/ic_add_field.png) \
        $$quote($$BASEDIR/assets/images/ic_add_group.png) \
        $$quote($$BASEDIR/assets/images/ic_attach.png) \
        $$quote($$BASEDIR/assets/images/ic_create_database.png) \
        $$quote($$BASEDIR/assets/images/ic_done.png) \
        $$quote($$BASEDIR/assets/images/ic_edit.png) \
        $$quote($$BASEDIR/assets/images/ic_edit_group.png) \
        $$quote($$BASEDIR/assets/images/ic_edit_key.png) \
        $$quote($$BASEDIR/assets/images/ic_email.png) \
        $$quote($$BASEDIR/assets/images/ic_empty_group.png) \
        $$quote($$BASEDIR/assets/images/ic_entry.png) \
        $$quote($$BASEDIR/assets/images/ic_entry_general.png) \
        $$quote($$BASEDIR/assets/images/ic_expired_item.png) \
        $$quote($$BASEDIR/assets/images/ic_history.png) \
        $$quote($$BASEDIR/assets/images/ic_lock.png) \
        $$quote($$BASEDIR/assets/images/ic_new_password.png) \
        $$quote($$BASEDIR/assets/images/ic_notes.png) \
        $$quote($$BASEDIR/assets/images/ic_quicklock.png) \
        $$quote($$BASEDIR/assets/images/ic_refresh.png) \
        $$quote($$BASEDIR/assets/images/ic_rename.png) \
        $$quote($$BASEDIR/assets/images/ic_save.png) \
        $$quote($$BASEDIR/assets/images/ic_search.png) \
        $$quote($$BASEDIR/assets/images/ic_timestamp.png) \
        $$quote($$BASEDIR/assets/images/ic_unlock.png) \
        $$quote($$BASEDIR/assets/images/ic_view_details.png) \
        $$quote($$BASEDIR/assets/main.qml) \
        $$quote($$BASEDIR/assets/pwicons-dark/0.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/1.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/10.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/11.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/12.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/13.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/14.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/15.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/16.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/17.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/18.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/19.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/2.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/20.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/21.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/22.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/23.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/24.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/25.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/26.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/27.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/28.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/29.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/3.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/30.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/31.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/32.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/33.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/34.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/35.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/36.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/37.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/38.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/39.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/4.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/40.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/41.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/42.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/43.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/44.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/45.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/46.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/47.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/48.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/49.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/5.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/50.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/51.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/52.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/53.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/54.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/55.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/56.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/57.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/58.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/59.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/6.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/60.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/61.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/62.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/63.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/64.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/65.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/66.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/67.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/68.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/7.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/8.png) \
        $$quote($$BASEDIR/assets/pwicons-dark/9.png)
}

config_pri_source_group1 {
    SOURCES += \
        $$quote($$BASEDIR/src/applicationui.cpp) \
        $$quote($$BASEDIR/src/crypto/CryptoManager.cpp) \
        $$quote($$BASEDIR/src/db/PwDatabase.cpp) \
        $$quote($$BASEDIR/src/db/PwEntry.cpp) \
        $$quote($$BASEDIR/src/db/PwGroup.cpp) \
        $$quote($$BASEDIR/src/db/PwUuid.cpp) \
        $$quote($$BASEDIR/src/db/v3/PwDatabaseV3.cpp) \
        $$quote($$BASEDIR/src/db/v3/PwEntryV3.cpp) \
        $$quote($$BASEDIR/src/db/v3/PwGroupV3.cpp) \
        $$quote($$BASEDIR/src/db/v3/PwStreamUtilsV3.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwDatabaseV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwDeletedObject.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwEntryV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwGroupV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwMetaV4.cpp) \
        $$quote($$BASEDIR/src/db/v4/PwStreamUtilsV4.cpp) \
        $$quote($$BASEDIR/src/main.cpp) \
        $$quote($$BASEDIR/src/ui/ActiveFrame.cpp) \
        $$quote($$BASEDIR/src/util/PasswordGenerator.cpp) \
        $$quote($$BASEDIR/src/util/ProgressObserver.cpp) \
        $$quote($$BASEDIR/src/util/Settings.cpp) \
        $$quote($$BASEDIR/src/util/TimedClipboard.cpp) \
        $$quote($$BASEDIR/src/util/Util.cpp)

    HEADERS += \
        $$quote($$BASEDIR/src/applicationui.hpp) \
        $$quote($$BASEDIR/src/crypto/CryptoManager.h) \
        $$quote($$BASEDIR/src/db/PwDatabase.h) \
        $$quote($$BASEDIR/src/db/PwEntry.h) \
        $$quote($$BASEDIR/src/db/PwGroup.h) \
        $$quote($$BASEDIR/src/db/PwIcon.h) \
        $$quote($$BASEDIR/src/db/PwUuid.h) \
        $$quote($$BASEDIR/src/db/v3/PwDatabaseV3.h) \
        $$quote($$BASEDIR/src/db/v3/PwEntryV3.h) \
        $$quote($$BASEDIR/src/db/v3/PwGroupV3.h) \
        $$quote($$BASEDIR/src/db/v3/PwStreamUtilsV3.h) \
        $$quote($$BASEDIR/src/db/v4/DefsV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwDatabaseV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwDeletedObject.h) \
        $$quote($$BASEDIR/src/db/v4/PwEntryV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwGroupV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwMetaV4.h) \
        $$quote($$BASEDIR/src/db/v4/PwStreamUtilsV4.h) \
        $$quote($$BASEDIR/src/ui/ActiveFrame.h) \
        $$quote($$BASEDIR/src/util/PasswordGenerator.h) \
        $$quote($$BASEDIR/src/util/ProgressObserver.h) \
        $$quote($$BASEDIR/src/util/Settings.h) \
        $$quote($$BASEDIR/src/util/TimedClipboard.h) \
        $$quote($$BASEDIR/src/util/Util.h)
}

INCLUDEPATH += $$quote($$BASEDIR/src/db/v4) \
    $$quote($$BASEDIR/src/util) \
    $$quote($$BASEDIR/src/db) \
    $$quote($$BASEDIR/src/db/v3) \
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
        $$quote($$BASEDIR/../src/db/v3/*.c) \
        $$quote($$BASEDIR/../src/db/v3/*.c++) \
        $$quote($$BASEDIR/../src/db/v3/*.cc) \
        $$quote($$BASEDIR/../src/db/v3/*.cpp) \
        $$quote($$BASEDIR/../src/db/v3/*.cxx) \
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
        $$quote($$BASEDIR/../assets/images/*.qs) \
        $$quote($$BASEDIR/../assets/pwicons-dark/*.qml) \
        $$quote($$BASEDIR/../assets/pwicons-dark/*.js) \
        $$quote($$BASEDIR/../assets/pwicons-dark/*.qs)

    HEADERS += \
        $$quote($$BASEDIR/../src/*.h) \
        $$quote($$BASEDIR/../src/*.h++) \
        $$quote($$BASEDIR/../src/*.hh) \
        $$quote($$BASEDIR/../src/*.hpp) \
        $$quote($$BASEDIR/../src/*.hxx)
}

TRANSLATIONS = $$quote($${TARGET}_ru.ts) \
    $$quote($${TARGET}.ts)
