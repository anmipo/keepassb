import bb.cascades 1.2
import bb.cascades.pickers 1.0
import bb.system 1.2
import org.keepassb 1.0

Container {
    property PwAttachment selectedAttachment
    property string savedFileName
    
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
            selectedAttachment = attachment;
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
                savedFileName = selectedFiles[0]; // full path
                console.log("savedFileName" + savedFileName);
                var success = selectedAttachment.saveContentToFile(savedFileName); 
                if (success) {
                    infoToast.body = qsTr("File saved")
                    infoToast.button.enabled = true;
                } else {
                    infoToast.body = qsTr("Could not save file")
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
            button.label: qsTr("Open") + Retranslate.onLocaleOrLanguageChanged
            onFinished: {
                if (value == SystemUiResult.ButtonSelection) {
                    app.invokeFile("file://" + savedFileName);
                }
            }
        }
    ]
}
