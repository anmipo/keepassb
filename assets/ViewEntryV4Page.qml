/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0
import "common.js" as Common

Page {
    property PwEntryV4 entry
    property string currentView
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    
    titleBar: TitleBar { 
        scrollBehavior: TitleBarScrollBehavior.Sticky
        kind: TitleBarKind.Segmented
        onSelectedValueChanged: {
            app.restartWatchdog();
            if (selectedValue)
                setCurrentView(selectedValue);
        }
        options: [
            Option {
                text: qsTr("General", "Title of a page which shows main/basic properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "general"
            },
            Option {
                text: qsTr("Extra", "Title of a page which shows additional/advanced properties of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "extra"
            },
            Option {
                text: qsTr("History", "Title of a page which lists the previous versions of an entry.") + Retranslate.onLocaleOrLanguageChanged
                value: "history"
            }
        ]
    }

    function setCurrentView(viewName) {
        var newView;
        var newViewComponent;
        switch (viewName) {
            case "general": 
                newView = viewEntryGeneralTab; 
                break;
            case "extra":   
                newView = viewEntryExtrasTab; 
                break;
            case "history":   
                newView = viewEntryHistoryTab;   
                break;
            default:
                console.log("WARN: unknown option");
                return;
        }
        currentView = viewName;
        if (entryContent.count() == 0) {
            entryContent.add(newView);
        } else {
            entryContent.replace(0, newView);
        }
    }

    Container {
        onTouchCapture: {
            app.restartWatchdog();
        }

        leftPadding: 10
        rightPadding: 10
        topPadding: 10
        bottomPadding: 10
        Container {
            id: entryContent
            // subpages will be dynamically inserted here
        }
    }
    attachedObjects: [
        ScrollView {
            id: viewEntryGeneralTab
            scrollRole: ScrollRole.Main
            Container {
                Container {
                    layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                    ImageView {
                        imageSource: "asset:///pwicons-dark/" + entry.iconId + ".png"
                        verticalAlignment: VerticalAlignment.Center
                    }
                    Label {
                        text: entry.title
                        multiline: true
                        textFormat: TextFormat.Plain
                        textStyle.base: SystemDefaults.TextStyles.PrimaryText
                        textStyle.color: Color.White
                        verticalAlignment: VerticalAlignment.Center
                        layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                    }
                }
                LabelTextButton {
                    id: usernameField
                    labelText: qsTr("User Name", "Label of the username field; refers to login information rather then person's own name.") + Retranslate.onLocaleOrLanguageChanged
                    valueText: entry.userName
                }
                LabelTextButton {
                    id: passwordField
                    labelText: qsTr("Password", "Label of the password field.") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_copy.png"
                    valueText: entry.password
                    passwordMasking: true
                }
                LabelTextButton {
                    id: urlField
                    labelText: qsTr("URL", "Label of the entry field containing a link/internet address.") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_copy.png"
                    valueText: entry.url
                    gestureHandlers: TapHandler {
                        onTapped: {
                            Qt.openUrlExternally(urlField.valueText);
                        }
                    } 
                }
                LabelTextButton {
                    id: notesText
                    labelText: qsTr("Notes", "Label of the entry field containing comments or additional text information.") + Retranslate.onLocaleOrLanguageChanged
                    valueText: entry.notes
                }
            }       
        },
        Container {
            id: viewEntryExtrasTab
            property PwAttachment selectedAttachment
            property string savedFileName
            ListView {
                id: entryExtraList
                scrollRole: ScrollRole.Main
                dataModel: entry.getExtraFieldsDataModel()
                visible: entry.extraSize > 0
                listItemComponents: [
                    ListItemComponent {
                        LabelTextButton {
                            labelText: ListItemData.name
                            valueText: ListItemData.value
                        }
                    }
                ]
            }
            Divider { 
                preferredHeight: 30
                visible: entry.extraSize > 0 
            }
            Header {
                title: qsTr("Attached Files", "Title of a list with attached files") + Retranslate.onLocaleOrLanguageChanged
            }
            Label {
                text: qsTr("There are no attached files.", "Explanation for the empty list of attached files.")
                visible: entry.attachmentCount == 0
            }
            ListView {
                id: entryFileList
                scrollRole: ScrollRole.Main
                dataModel: entry.getAttachmentsDataModel()
                visible: entry.attachmentCount > 0
                listItemComponents: [
                    ListItemComponent {
                        StandardListItem {
                            title: ListItemData.name
                            description: qsTr("%n byte(s)", "File size in bytes; please specify the required plural forms.", ListItemData.size) + Retranslate.onLocaleOrLanguageChanged
                        }
                    }
                ]
                onTriggered: {
                    var attachment = dataModel.data(indexPath);
                    selectedAttachment = attachment;
                    saveAttachmentFilePicker.defaultSaveFileNames = [attachment.name];
                    saveAttachmentFilePicker.open();
                }
                attachedObjects: [
                    FilePicker {
                        id: saveAttachmentFilePicker
                        title: qsTr("Save File", "Title of a file saving dialog.") + Retranslate.onLocaleOrLanguageChanged
                        mode: FilePickerMode.Saver
                        type: FileType.Other
                        allowOverwrite: true
                        onFileSelected: {
                            savedFileName = selectedFiles[0]; // full path
                            var success = selectedAttachment.saveContentToFile(savedFileName); 
                            if (success) {
                                attachmentFileToast.body = qsTr("File saved", "A confirmation message once the file has been successfully saved")
                                attachmentFileToast.button.enabled = true;
                            } else {
                                attachmentFileToast.body = qsTr("Could not save file", "Error message")
                                attachmentFileToast.button.enabled = false;
                            }
                            attachmentFileToast.show();
                        } 
                        onCanceled: {
                            // nothing to do here
                        }
                    },
                    SystemToast {
                        id: attachmentFileToast
                        button.label: qsTr("Open", "A button/action which opens (or launches) a file (see related error message with reference INVOKE_ATTACHMENT)") + Retranslate.onLocaleOrLanguageChanged
                        onFinished: {
                            if (value == SystemUiResult.ButtonSelection) {
                                app.invokeFile("file://" + savedFileName);
                            }
                        }
                    }
                ]
            }
        },
        ScrollView {
            id: viewEntryHistoryTab
            Container {
                LabelTextButton { 
                    labelText: qsTr("Expiry Date", "Label of a field with date and time when the entry will no longer be valid. 'Never' is also a possible value.") + Retranslate.onLocaleOrLanguageChanged
                    valueText: entry.expires ? Common.timestampToString(entry.expiryTime) : qsTr("Never", "Expiry Date of the entry which does not expire.")
                }
                LabelTextButton { 
                    labelText: qsTr("Creation Date", "Label of a field with entry creation date and time") + Retranslate.onLocaleOrLanguageChanged
                    valueText: Common.timestampToString(entry.creationTime)  
                }
                LabelTextButton { 
                    labelText: qsTr("Last Modification Date", "Label of a field with entry's last modification date and time") + Retranslate.onLocaleOrLanguageChanged
                    valueText: Common.timestampToString(entry.lastModificationTime)  
                }
                LabelTextButton { 
                    labelText: qsTr("Last Access Date", "Label of a field with date and time when the entry was last accessed/viewed") + Retranslate.onLocaleOrLanguageChanged
                    valueText: Common.timestampToString(entry.lastAccessTime)
                }
                Header {
                    title: qsTr("Previous Versions", "Header of a list with previous versions/revisions of an entry.") + Retranslate.onLocaleOrLanguageChanged
                }
                Label {
                    text: qsTr("There are no previous versions.", "Explanation for the empty list of previous entry versions/revisions.")
                    visible: entry.historySize == 0
                }
                ListView {
                    id: entryHistoryList
                    visible: entry.historySize > 0
                    scrollRole: ScrollRole.Main
                    preferredHeight: 360
                    dataModel: entry.getHistoryDataModel()
                    onTriggered: {
                        var item = dataModel.data(indexPath);
                        var viewHistoryEntryPage = Qt.createComponent("ViewEntryV4Page.qml");
                        var historyEntryPage = viewHistoryEntryPage.createObject(null, {"entry": item});
                        naviPane.push(historyEntryPage);
                    }
                    function updateHeight(itemHeight) {
                        entryHistoryList.maxHeight = itemHeight * entry.historySize;
                    }
                    listItemComponents: [
                        ListItemComponent {
                            StandardListItem {
                                id: entryHistoryListItem
                                title: ListItemData.title
                                description: ListItemData.lastModificationTime.toString()
                                imageSpaceReserved: true
                                imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                                attachedObjects: [
                                    // ensures the list is large enough to fit all the entries, but no more than that
                                    LayoutUpdateHandler {
                                        onLayoutFrameChanged: {
                                            entryHistoryListItem.ListItem.view.updateHeight(layoutFrame.height);
                                        }
                                    }
                                ]
                            }
                        }
                    ]
                }
            }
        }
    ]
}
