/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0

Page {
    property PwEntryV4 data
    property string currentView
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    
    titleBar: TitleBar { 
        title: data.title
        visibility: ChromeVisibility.Overlay
        scrollBehavior: TitleBarScrollBehavior.NonSticky
    }

    actions: [
        ActionItem {
            title: qsTr("General") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///pwicons/0.png" //TODO make a dedicated icon
//            enabled: (currentView != "general")
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: setCurrentView("general")
        },
        ActionItem {
            title: qsTr("Extras (%1)").arg(data.extraSize) + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_view_details.png"
            enabled: (data.extraSize > 0)
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: setCurrentView("extra")
        },
        ActionItem {
            title: qsTr("Files (%1)").arg(data.attachmentCount) + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_attach.png"
            enabled: (data.attachmentCount > 0)
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: setCurrentView("files")
        },
        ActionItem {
            title: qsTr("History (%1)").arg(data.historySize) + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_history.png"
            enabled: (data.historySize > 0)
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: setCurrentView("history")
        },
        ActionItem {
            title: qsTr("Timestamps") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///pwicons/39.png" //TODO make a dedicated icon
//            enabled: (currentView != "timestamps")
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: setCurrentView("timestamps")
        }
    ]
    
    function setCurrentView(viewName) {
        app.restartWatchdog();
        var newView;
        switch (viewName) {
            case "general": newView = viewEntryGeneral; break;
            case "extra":   newView = viewEntryExtra;   break;
            case "files":   newView = viewEntryFiles;   break;
            case "timestamps": newView = viewEntryTimestamps; break;
            case "history":   newView = viewEntryHistory;   break;
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

    onCreationCompleted: {
        setCurrentView("general");
    }

    Container {
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
            id: viewEntryGeneral
            scrollRole: ScrollRole.Main
            Container {
                LabelTextButton {
                    id: usernameField
                    labelText: qsTr("User name") + Retranslate.onLocaleOrLanguageChanged
                    valueText: data.userName
                }
                LabelTextButton {
                    id: passwordField
                    labelText: qsTr("Password") + Retranslate.onLocaleOrLanguageChanged
//                    inputMode: TextFieldInputMode.Password
                    imageSource: "asset:///images/ic_copy_password.png"
                    valueText: data.password
                    passwordMasking: true
                }
                LabelTextButton {
                    id: urlField
                    labelText: qsTr("URL") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_copy_link.png"
                    valueText: data.url
                    gestureHandlers: TapHandler {
                        onTapped: {
                            Qt.openUrlExternally(urlField.valueText);
                        }
                    } 
                }
                LabelTextButton {
                    id: notesText
                    //editable: false
                    //text: data.notes
                    labelText: qsTr("Notes") + Retranslate.onLocaleOrLanguageChanged
                    valueText: data.notes
                }
            }       
        },
        Container {
            id: viewEntryExtra
            ListView {
                id: entryExtraList
                scrollRole: ScrollRole.Main
                dataModel: data.getExtraFieldsDataModel()
                accessibility.name: qsTr("Extra fields") + Retranslate.onLocaleOrLanguageChanged
                listItemComponents: [
                    ListItemComponent {
                        LabelTextButton {
                            labelText: ListItemData.name
                            valueText: ListItemData.value
                        }
                    }
                ]
            }
        },
        Container {
            id: viewEntryFiles
            property PwAttachment selectedAttachment
            ListView {
                
                id: entryFileList
                scrollRole: ScrollRole.Main
                dataModel: data.getAttachmentsDataModel()
                accessibility.name: qsTr("Attachments") + Retranslate.onLocaleOrLanguageChanged
                listItemComponents: [
                    ListItemComponent {
                        StandardListItem {
                            title: ListItemData.name
                            description: qsTr("%n byte(s)", "", ListItemData.size) + Retranslate.onLocaleOrLanguageChanged
                        }
                    }
                ]
                onTriggered: {
                    var attachment = dataModel.data(indexPath);
                    viewEntryFiles.selectedAttachment = attachment;
                    console.log("Attachment name=" + attachment.name + ", size: " + attachment.size);
                    saveAttachmentFilePicker.defaultSaveFileNames = [attachment.name];
                    saveAttachmentFilePicker.open();
                }
            }
            attachedObjects: [
                FilePicker {
                    id: saveAttachmentFilePicker
                    title: qsTr("Save attached file") + Retranslate.onLocaleOrLanguageChanged
                    mode: FilePickerMode.Saver
                    type: FileType.Other
                    allowOverwrite: true
                    onFileSelected: {
                        var success = viewEntryFiles.selectedAttachment.saveContentToFile(selectedFiles[0]); // actual path
                        if (success) {
                            infoToast.body = qsTr("File saved")
                        } else {
                            infoToast.body = qsTr("Could not save file")
                        }
                        infoToast.show();
                    } 
                    onCanceled: {
                        // ignore this
                    }
                },
                SystemToast {
                    id: infoToast
                }
            ]
        },
        Container {
            id: viewEntryHistory
            Header {
                title: qsTr("Entry history") + Retranslate.onLocaleOrLanguageChanged
            }
            ListView {
                id: entryHistoryList
                scrollRole: ScrollRole.Main
                visible: (data.historySize > 0)
                dataModel: data.getHistoryDataModel()
                onTriggered: {
                    var item = dataModel.data(indexPath);
                    var viewHistoryEntryPage = Qt.createComponent("ViewEntryV4Page.qml");
                    var historyEntryPage = viewHistoryEntryPage.createObject(null, {"data": item});
                    naviPane.push(historyEntryPage);
                }
                listItemComponents: [
                    ListItemComponent {
                        StandardListItem {
                            title: ListItemData.title
                            description: ListItemData.lastModificationTime.toString()
                            imageSpaceReserved: true
                            imageSource: "asset:///pwicons/" + ListItemData.iconId + ".png"
                            accessibility.name: qsTr("Entry") + Retranslate.onLocaleOrLanguageChanged + ListItemData.title
                        }
                    }
                ]
            }
        },
        Container {
            id: viewEntryTimestamps
            LabelTextButton { 
                labelText: qsTr("Creation Date") + Retranslate.onLocaleOrLanguageChanged
                valueText: data.creationTime.toString()  
            }
            LabelTextButton { 
                labelText: qsTr("Last Modification Date") + Retranslate.onLocaleOrLanguageChanged
                valueText: data.lastModificationTime.toString()  
            }
            LabelTextButton { 
                labelText: qsTr("Last Access Date") + Retranslate.onLocaleOrLanguageChanged
                valueText: data.lastAccessTime.toString()
            }
            LabelTextButton { 
                labelText: qsTr("Expiry Date") + Retranslate.onLocaleOrLanguageChanged
                valueText: data.expiryTime.toString()
            }
        }
    ]
}
