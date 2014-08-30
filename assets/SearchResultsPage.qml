import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

Page {
    property PwSearchResultDataModel searchResult
    
    titleBar: TitleBar {
        title: qsTr("Search Results", "Title of the page which lists search results") + Retranslate.onLocaleOrLanguageChanged 
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
                
                var viewEntryPageComponent;
                if (Qt.database.getFormatVersion() == 3) {
                    viewEntryPageComponent = Qt.createComponent("ViewEntryV3Page.qml");
                } else {
                    viewEntryPageComponent = Qt.createComponent("ViewEntryV4Page.qml");
                }
                var viewEntryPage = viewEntryPageComponent.createObject(null, {"data": entry});
                naviPane.push(viewEntryPage);
            }
            listItemComponents: [
                ListItemComponent {
                    StandardListItem {
                        title: ListItemData.title
                        //description: Common.getEntryDescription(ListItemData)
                        description: qsTr("Group: %1", "Describes the group of the selected entry. Example: 'Group:  Internet'").arg(ListItemData.parentGroup.name)
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
                text: qsTr("Nothing found", "Description of an empty search result") + Retranslate.onLocaleOrLanguageChanged
                textStyle.base: SystemDefaults.TextStyles.BodyText
                horizontalAlignment: HorizontalAlignment.Center
            }
        }
    }
}
