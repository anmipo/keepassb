import bb.cascades 1.2
import org.keepassb 1.0

Sheet {
    id: iconPickerSheet
    signal iconPicked(int iconId)
    
    onCreationCompleted: {
        database.dbLocked.connect(function() {
                // close without saving when DB is being locked
                iconPickerSheet.close();
        });
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