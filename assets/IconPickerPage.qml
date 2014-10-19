import bb.cascades 1.2
import org.keepassb 1.0

Sheet {
    id: iconPickerSheet
    signal iconPicked(int iconId)
    
    onCreationCompleted: {
        // close without saving when DB is being locked
        database.dbLocked.connect(_close);
    }
    onClosed: {
        database.dbLocked.disconnect(_close);
        destroy();
    }
    function _close() {
        close();
    }

    Page {
        titleBar: TitleBar {
            title: qsTr("Choose Icon", "Title of a page for selecting an icon image") + Retranslate.onLocaleOrLanguageChanged
            dismissAction: ActionItem {
                title: qsTr("Cancel", "A button/action") + Retranslate.onLocaleOrLanguageChanged
                onTriggered: {
                    app.restartWatchdog();
                    close();
                }
            }
        }
        Container {
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10
            onTouchCapture: {
                app.restartWatchdog();
            }
            ListView {
                function listItemClicked(iconId) {
                    iconPicked(iconId);
                    iconPickerSheet.close();
                }
                layout: FlowListLayout {
                    headerMode: ListHeaderMode.None
                    orientation: LayoutOrientation.TopToBottom
                }
                dataModel: XmlDataModel {
                    source: "asset:///icons_data_model.xml"
                }
                listItemComponents: [
                    ListItemComponent {
                        type: "icon"
                        ImageButton {
                            topMargin: 20
                            leftMargin: 20
                            defaultImageSource: "asset:///pwicons-dark/" + ListItemData.img
                            pressedImageSource: "asset:///pwicons-dark/" + ListItemData.img
                            onClicked: {
                                ListItem.view.listItemClicked(ListItemData.id);
                            }                            
                        }
                    }
                ]
            }
        }
    }
}