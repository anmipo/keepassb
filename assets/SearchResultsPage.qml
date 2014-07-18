import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

Page {
    property PwSearchResultDataModel searchResult
    
    titleBar: TitleBar {
        title: qsTr("Search results") + Retranslate.onLocaleOrLanguageChanged 
    }
    Container {
        onTouchCapture: {
            app.restartWatchdog();
        }

        layout: DockLayout { }
        ListView {
            id: listView
            visible: searchResult.hasChildren([])
            dataModel: searchResult
            onTriggered: {
                var entry = searchResult.data(indexPath);
                var viewEntryPageComponent = Qt.createComponent("ViewEntryV4Page.qml");
                var viewEntryPage = viewEntryPageComponent.createObject(null, {"data": entry});
                naviPane.push(viewEntryPage);
            }
            listItemComponents: [
                ListItemComponent {
                    StandardListItem {
                        title: ListItemData.title
                        //description: Common.getEntryDescription(ListItemData)
                        description: qsTr("Group: ") + ListItemData.getParentGroup().name
                        imageSpaceReserved: true
                        imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                    }
                }
            ]
        }
        Container {
            layout: StackLayout { }
            visible: !searchResult.hasChildren([])
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
            ImageView {
                imageSource: "asset:///images/ic_search.png"
                horizontalAlignment: HorizontalAlignment.Center
            }
            Label {
                text: qsTr("Nothing found") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.BodyText
                horizontalAlignment: HorizontalAlignment.Center
            }
        }
    }
}
