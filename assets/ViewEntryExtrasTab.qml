/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import bb.cascades.pickers 1.0
import org.keepassb 1.0

Container {
    property PwAttachment selectedAttachment
    property string savedFileName
    property bool hasExtraStrings: (database.getFormatVersion() == 4)
    
    // called when the user wants to attach a file
    function onAddAttachment() {
        console.log("onAddAttachment");
        addAttachmentFilePicker.open();
    }

    // appends given file to the current entry
    function attachFile(fileName) {
        // when we are here, we already have user's permission to replace current attachment if necessary 
        var success = entry.attachFile(fileName); //append (or replace) file to the entry
        addAttachmentToast.success = success;
        addAttachmentToast.show();
        if (success) {
            app.restartWatchdog();
            database.save();
        }
    }
    
    ListView {
        id: entryExtraList
        dataModel: hasExtraStrings ? entry.getExtraFieldsDataModel() : null
        visible: hasExtraStrings && (entry.extraSize > 0)
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
        visible: hasExtraStrings && (entry.extraSize > 0) 
    }
    Header {
        title: qsTr("Attached Files", "Title of a list with attached files") + Retranslate.onLocaleOrLanguageChanged
    }
    Label {
        text: qsTr("There are no attached files.", "Explanation for the empty list of attached files.")
        visible: (entryFileList.dataModel.size == 0)
    }
    ListView {
        property bool entryEditable: database.isEditable() && !entry.deleted
        
        function removeAttachmentAt(index) {
            entry.renewTimestamps();
            entry.backupState();
            dataModel.removeAt(index);
            
            app.restartWatchdog();
            database.save();
        }
        
        id: entryFileList
        dataModel: entry.getAttachmentsDataModel()
        visible: (entryFileList.dataModel.size > 0)
        listItemComponents: [
            ListItemComponent {
                StandardListItem {
                    id: entryFileListItem
                    title: ListItemData.name
                    imageSource: "asset:///images/ic_attach.png" 
                    description: qsTr("%n byte(s)", "File size in bytes; please specify the required plural forms.", ListItemData.size) + Retranslate.onLocaleOrLanguageChanged
                    contextActions: ActionSet {
                        title: ListItemData.name
                        actions: [
                            DeleteActionItem {
                                title: qsTr("Remove File", "A button/action to remove/delete the selected file") + Retranslate.onLocaleOrLanguageChanged
                                enabled: entryFileListItem.ListItem.view.entryEditable
                                onTriggered: {
                                    var selIndex = entryFileListItem.ListItem.indexInSection;
                                    var listView = entryFileListItem.ListItem.view;
                                    listView.removeAttachmentAt(selIndex);
                                }
                            }
                        ]
                    }
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
            FilePicker {
                id: addAttachmentFilePicker
                title: qsTr("Select File", "Title of a file selection dialog.") + Retranslate.onLocaleOrLanguageChanged
                mode: FilePickerMode.Picker
                type: FileType.Other
                onFileSelected: {
                    var selectedFileName = selectedFiles[0]; // full path
                    
                    // V3 supports only one attachment per entry
                    if ((database.getFormatVersion() == 3) && (entryFileList.dataModel.size > 0)) {
                        singleAttachmentWarningDialog.fileName = selectedFileName;
                        singleAttachmentWarningDialog.show();
                    } else {
                        attachFile(selectedFileName);
                    }
                } 
                onCanceled: {
                    console.log("Attachment cancelled");
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
            },
            SystemToast {
                property bool success // chooses appropriate message depending on attachment outcome
                id: addAttachmentToast
                body: success ? 
                        qsTr("File attached", "A confirmation message once the file has been successfully attached to the entry") :
                        qsTr("Could not attach file", "Error message")
            },
            SystemDialog {
                property string fileName
                
                id: singleAttachmentWarningDialog
                title: qtTr("Warning", "Title of an window with important question") + Retranslate.onLocaleOrLanguageChanged
                body: qsTr("KDB format allows only one attachment per entry. Replace the current attachment?", "KDB is a file format, do not translate") + Retranslate.onLocaleOrLanguageChanged
                confirmButton.label: qsTr("Replace", "A button/action to confirm that the old attachment should be replaced by the new one") + Retranslate.onLocaleOrLanguageChanged
                onFinished: {
                    if (value == SystemUiResult.ConfirmButtonSelection) {
                        attachFile(fileName);
                    } else {
                        console.log("Attachment cancelled");
                    }
                }
            }
        ]
    }
}
