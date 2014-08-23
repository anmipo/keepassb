/*
 * Copyright (c) 2014 Andrei Popleteev
 */

import bb.cascades 1.2
import bb.device 1.2
import bb.system 1.2
import org.keepassb 1.0

Page {
    property PwEntryV3 data
    property string currentView
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    
    titleBar: TitleBar { 
        visibility: ChromeVisibility.Overlay
        scrollBehavior: TitleBarScrollBehavior.NonSticky
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                layout: StackLayout { orientation: LayoutOrientation.LeftToRight }
                leftPadding: 10
                rightPadding: 10
                ImageView {
                    imageSource: "asset:///pwicons-dark/" + data.iconId + ".png"
                    verticalAlignment: VerticalAlignment.Center
                }
                Label {
                    text: data.title
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                    textStyle.color: Color.White
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties { spaceQuota: 1 }
                }
            }
            
            expandableArea.indicatorVisibility: (data.title.length > 30) ? TitleBarExpandableAreaIndicatorVisibility.Visible : TitleBarExpandableAreaIndicatorVisibility.Hidden
            expandableArea.toggleArea: TitleBarExpandableAreaToggleArea.EntireTitleBar
            expandableArea.content: TextArea {
                editable: false
                text: data.title
                textFormat: TextFormat.Plain
                backgroundVisible: false
                autoSize.maxLineCount: 3
            }
        }
    }

    actions: [
        ActionItem {
            title: qsTr("General") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_entry_general.png"
//            enabled: (currentView != "general")
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: setCurrentView("general")
        },
        ActionItem {
            title: qsTr("Files (%1)").arg(data.attachmentCount) + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_attach.png"
            enabled: (data.attachmentCount > 0)
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: setCurrentView("files")
        },
        ActionItem {
            title: qsTr("Timestamps") + Retranslate.onLocaleOrLanguageChanged
            imageSource: "asset:///images/ic_timestamp.png"
//            enabled: (currentView != "timestamps")
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: setCurrentView("timestamps")
        }
    ]
    
    function setCurrentView(viewName) {
        var newView;
        var newViewComponent;
        switch (viewName) {
            case "general": 
                newView = viewEntryGeneral; 
                break;
            case "files":   
                newView = viewEntryFilesComponent.createObject();
                break;
            case "timestamps": 
                newView = viewEntryTimestamps;
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

    onCreationCompleted: {
        setCurrentView("general");
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
        ComponentDefinition {
            id: viewEntryFilesComponent
            source: "ViewEntryFiles.qml"
        },
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
                    imageSource: "asset:///images/ic_copy.png"
                    valueText: data.password
                    passwordMasking: true
                }
                LabelTextButton {
                    id: urlField
                    labelText: qsTr("URL") + Retranslate.onLocaleOrLanguageChanged
                    imageSource: "asset:///images/ic_copy.png"
                    valueText: data.url
                    gestureHandlers: TapHandler {
                        onTapped: {
                            Qt.openUrlExternally(urlField.valueText);
                        }
                    } 
                }
                LabelTextButton {
                    id: notesText
                    labelText: qsTr("Notes") + Retranslate.onLocaleOrLanguageChanged
                    valueText: data.notes
                }
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
