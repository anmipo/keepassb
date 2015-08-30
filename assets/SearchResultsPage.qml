import bb.cascades 1.2
import bb.system 1.2
import org.keepassb 1.0
import "common.js" as Common

PageWithWatchdog {
    id: searchPage
    property bool autofocus: true
    property string searchQuery
    property PwSearchResultDataModel searchResult: database.searchResult;
    property int searchResultSize: -1 // updated after each search; "-1" is for empty query

    function performAutofocus() {
        searchField.requestFocus();
    }

    // Runs search with the latest query
    function performSearch() {
        if (searchQuery.length > 0) {
            database.search(searchQuery);
            searchResultSize = searchResult.size();
        } else {
            searchResultSize = -1;
        }
    }
    function showEntry(entry) {
        var viewEntryPageComponent = Qt.createComponent("ViewEntryPage.qml");
        var viewEntryPage = viewEntryPageComponent.createObject(null, {"entry": entry});
        naviPane.push(viewEntryPage);
    }
    function showEditEntryDialog(selEntry) {
        var editEntryPageComponent = Qt.createComponent("EditEntryPage.qml");
        var editEntrySheet = editEntryPageComponent.createObject(viewGroupPage, {"entry": selEntry});
        editEntrySheet.open();
        editEntrySheet.autofocus()
    }
    function confirmDeleteEntry(selEntry) {
        deleteEntryConfirmationDialog.targetEntry = selEntry;
        deleteEntryConfirmationDialog.show();
    }

    onSearchQueryChanged: {
        performSearch();
    }
    
    titleBar: TitleBar {
        title: qsTr("Search Results", "Title of the page which lists search results") + Retranslate.onLocaleOrLanguageChanged 
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                leftPadding: 10
                rightPadding: 10
                verticalAlignment: VerticalAlignment.Fill
                TextField {
                    id: searchField
                    hintText: qsTr("Enter search query", "Hint text for the search query input field, invites the user to type some text") + Retranslate.onLocaleOrLanguageChanged
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                    clearButtonVisible: true 
                    inputMode: TextFieldInputMode.Text
                    input.submitKey: SubmitKey.Done
                    input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Lose
                    input.onSubmitted: {
                        if (searchResultSize == 1) {
                            // one result - open immediately
                            var entry = searchResult.data(searchResult.first());
                            showEntry(entry);
                        } else if (searchResultSize > 1) {
                            // many results - proceed with selection 
                            searchResultsList.requestFocus();
                        }
                    }
                    validator: Validator {
                        mode: ValidationMode.Delayed
                        delay: 300
                        onValidate: {
                            app.restartWatchdog();
                            searchQuery = searchField.text;                            
                        }
                    }
                }
                Button {
                    text: qsTr("Cancel", "Button/action to cancel/hide search query field") + Retranslate.onLocaleOrLanguageChanged
                    verticalAlignment: VerticalAlignment.Center
                    preferredWidth: 50
                    onClicked: {
                        searchField.text = "";
                        naviPane.pop();
                    }
                }
            }
            expandableArea.indicatorVisibility: TitleBarExpandableAreaIndicatorVisibility.Hidden
        }
    }
    paneProperties: NavigationPaneProperties {
        backButton: ActionItem {
            id: backButton
            onTriggered: {
                naviPane.pop();
            }
        } 
    }
    Container {
        layout: DockLayout { }
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Fill
        Label {
            text: qsTr("Nothing found", "Description of an empty search result") + Retranslate.onLocaleOrLanguageChanged
            visible: (searchResultSize == 0)
            verticalAlignment: VerticalAlignment.Top
            horizontalAlignment: HorizontalAlignment.Left
            textStyle.base: SystemDefaults.TextStyles.BodyText
            multiline: true
        }
        ListView {
            id: searchResultsList
            scrollRole: ScrollRole.Main

            function _showEditEntryDialog(selEntry) {
                var editEntryPageComponent = Qt.createComponent("EditEntryPage.qml");
                var editEntrySheet = editEntryPageComponent.createObject(viewGroupPage, {"entry": selEntry});
                editEntrySheet.open();
                editEntrySheet.autofocus();
            }
            function _confirmDeleteEntry(selEntry) {
                deleteEntryConfirmationDialog.targetEntry = selEntry;
                deleteEntryConfirmationDialog.show();
            }
            
            visible: (searchResultSize > 0)
            dataModel: searchResult
            onTriggered: {
                if (searchResult.itemType(indexPath) != "item")
                    return;
                
                var entry = searchResult.data(indexPath);
                showEntry(entry);
            }
            layout: StackListLayout {
                headerMode: ListHeaderMode.Sticky
            }
            listItemComponents: [
                ListItemComponent {
                    type: "header"
                    Header {
                        title: ListItemData
                    }
                },
                ListItemComponent {
                    type: "item"
                    EntryItem {
                        id: searchResultListEntryItem
                        onEditRequested: searchResultListEntryItem.ListItem.view._showEditEntryDialog(selectedEntry)
                        onDeleteRequested: searchResultListEntryItem.ListItem.view._confirmDeleteEntry(selectedEntry)
                    }
                }
            ]
        }
        Container {
            layout: StackLayout { }
            visible: (searchResultSize <= 0)
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
            ImageView {
                imageSource: "asset:///images/search_splash.png"
                horizontalAlignment: HorizontalAlignment.Center
            }
        }
    }
    attachedObjects: [
        SystemDialog {
            property variant targetEntry
            id: deleteEntryConfirmationDialog
            title: qsTr("Delete Entry", "Title of a delete confirmation dialog") + Retranslate.onLocaleOrLanguageChanged
            body: qsTr("Really delete this entry?", "A confirmation dialog for deleting entry") + Retranslate.onLocaleOrLanguageChanged
            confirmButton.label: qsTr("Delete", "A button/action to confirm deletion of an entry") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                if (value == SystemUiResult.ConfirmButtonSelection) {
                    Common.deleteEntry(targetEntry);
                    // refresh the ListView, otherwise it might crash
                    searchResult.itemsChanged(DataModelChangeType.AddRemove, 0);
                }
            }
        }
    ]
}
