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
        visible: entry.attachmentCount == 0
    }
    ListView {
        id: entryFileList
        dataModel: entry.getAttachmentsDataModel()
        visible: entry.attachmentCount > 0
        listItemComponents: [
            ListItemComponent {
                StandardListItem {
                    title: ListItemData.name
                    imageSource: "asset:///images/ic_attach.png" 
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
}
