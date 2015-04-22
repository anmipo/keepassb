import bb.cascades 1.2
import org.keepassb 1.0
import "common.js" as Common

PageWithWatchdog {
    id: searchResultsPage
    property string searchQuery
    property PwSearchResultDataModel searchResult: database.searchResult;

    // Runs search with the latest query
    function performSearch() {
        database.search(searchQuery);
        //searchResult = database.searchResult;
    }
        
    onSearchQueryChanged: {
        performSearch();
    }
    
    titleBar: TitleBar {
        title: qsTr("Search Results", "Title of the page which lists search results") + Retranslate.onLocaleOrLanguageChanged 
    }
    Container {
        layout: DockLayout { }
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill
        ListView {
            id: listView
            visible: searchResult.hasChildren([])
            dataModel: searchResult
            onTriggered: {
                var entry = searchResult.data(indexPath);
                
                var viewEntryPageComponent = Qt.createComponent("ViewEntryPage.qml");
                var viewEntryPage = viewEntryPageComponent.createObject(null, {"entry": entry});
                naviPane.push(viewEntryPage);
            }
            listItemComponents: [
                ListItemComponent {
                    GroupListItem {
                        itemType: "entry"
                        title: ListItemData.title
                        description: qsTr("Group: %1", "Describes the group of the selected entry. Example: 'Group:  Internet'").arg(ListItemData.parentGroup.name)
                        imageSource: ListItemData.expired ? "asset:///images/ic_expired_item.png" : "asset:///pwicons/" + ListItemData.iconId + ".png"
                        contextActions: ActionSet {
                            title: ListItemData.title
                            actions: [
                                ActionItem {
                                    title: qsTr("Multi-Copy", "A button/action which copies several values into clipboard (also see 'Multi-Copy' in thesaurus).") + Retranslate.onLocaleOrLanguageChanged
                                    imageSource: "asset:///images/ic_multi_copy.png"
                                    onTriggered: {
                                        Common.performMultiCopy(ListItemData);
                                    }
                                },
                                ActionItem {
                                    title: qsTr("Copy User Name", "A button/action which copies user_name value to the clipboard. Here, 'User Name' refers to login information rather that person's own name.") + Retranslate.onLocaleOrLanguageChanged
                                    imageSource: "asset:///images/ic_copy_username.png"
                                    onTriggered: {
                                        Qt.app.copyWithTimeout(ListItemData.userName);
                                    }
                                },
                                ActionItem {
                                    title: qsTr("Copy Password", "A button/action which copies password password value to the clipborad.") + Retranslate.onLocaleOrLanguageChanged
                                    imageSource: "asset:///images/ic_copy_password.png"
                                    onTriggered: {
                                        Qt.app.copyWithTimeout(ListItemData.password);
                                    }
                                }
                            ]
                        }
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
