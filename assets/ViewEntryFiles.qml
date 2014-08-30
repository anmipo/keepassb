import bb.cascades 1.2
import bb.cascades.pickers 1.0
import bb.system 1.2
import org.keepassb 1.0

Container {
    property PwAttachment selectedAttachment
    property string savedFileName

    Header {
        title: qsTr("Attachments", "Title for a list of attached files") + Retranslate.onLocaleOrLanguageChanged
    }    
    ListView {
        id: entryFileList
        scrollRole: ScrollRole.Main
        dataModel: data.getAttachmentsDataModel()
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
                    infoToast.body = qsTr("File saved", "A confirmation message once the file has been successfully saved")
                    infoToast.button.enabled = true;
                } else {
                    infoToast.body = qsTr("Could not save file", "Error message")
                    infoToast.button.enabled = false;
                }
                infoToast.show();
            } 
            onCanceled: {
                // ignore this
            }
        },
        SystemToast {
            id: infoToast
            button.label: qsTr("Open", "A button/action which opens (or launches) a file (see related error message with reference INVOKE_ATTACHMENT)") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                if (value == SystemUiResult.ButtonSelection) {
                    app.invokeFile("file://" + savedFileName);
                }
            }
        }
    ]
}
