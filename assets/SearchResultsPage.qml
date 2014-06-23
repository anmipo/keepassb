import bb.cascades 1.2
import org.keepassb 1.0

Page {
    property PwSearchResultDataModel searchResult
    
    titleBar: TitleBar {
        title: qsTr("Search results") + Retranslate.onLocaleOrLanguageChanged 
    }

    Container {
        layout: DockLayout { }
        ListView {
            id: listView
            visible: searchResult.hasChildren([])
            dataModel: searchResult
            onTriggered: {
                console.log("indexPath: " + indexPath)
                var entry = searchResult.data(indexPath);
                console.log("entry: " + entry)
                var viewEntryPageComponent = Qt.createComponent("ViewEntryV4Page.qml");
                var viewEntryPage = viewEntryPageComponent.createObject(null, {"data": entry});
                naviPane.push(viewEntryPage);
            }
            listItemComponents: [
                ListItemComponent {
                    StandardListItem {
                        title: ListItemData.title
                        description: ListItemData.userName
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
