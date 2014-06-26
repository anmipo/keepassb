import bb.cascades 1.2

Page {
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        searchInDeleted.checked = appSettings.searchInDeleted;
        clipboardTimeout.selectedIndex = appSettings.clipboardTimeoutIndex;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
        clipboardTimeout.selectedIndexChanged.connect(appSettings.setClipboardTimeoutIndex);
    }
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.TopToBottom
        }
        Container {
            layout: StackLayout { 
                orientation: LayoutOrientation.LeftToRight
            }
            horizontalAlignment: HorizontalAlignment.Fill
            StandardListItem {
                horizontalAlignment: HorizontalAlignment.Left
                verticalAlignment: VerticalAlignment.Center
                title: qsTr("Search in Recycle Bin") + Retranslate.onLocaleOrLanguageChanged
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 1
                }
            }
            ToggleButton {
                id: searchInDeleted
                horizontalAlignment: HorizontalAlignment.Right
                verticalAlignment: VerticalAlignment.Center
            }
        }
        DropDown {
            id: clipboardTimeout
            title: qsTr("Clear clipboard") + Retranslate.onLocaleOrLanguageChanged
            // values of these options must be in sync with Settings::CLIPBOARD_TIMEOUT_INDEX_TO_SECONDS
            Option {
                text: qsTr("Never") + Retranslate.onLocaleOrLanguageChanged
                value: -1;
            }
            Option {
                text: qsTr("after 10 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 10;
            }
            Option {
                text: qsTr("after 30 seconds") + Retranslate.onLocaleOrLanguageChanged
                value: 30;
            }
            Option {
                text: qsTr("after 1 minute") + Retranslate.onLocaleOrLanguageChanged
                value: 60;
            }
        }
    }
}