import bb.cascades 1.2

Page {
    titleBar: TitleBar {
        title: qsTr("Settings") + Retranslate.onLocaleOrLanguageChanged
    }
    onCreationCompleted: {
        searchInDeleted.checked = appSettings.searchInDeleted;
        searchInDeleted.checkedChanged.connect(appSettings.setSearchInDeleted);
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
    }
}