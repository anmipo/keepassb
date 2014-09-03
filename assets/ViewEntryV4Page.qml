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
        ViewEntryGeneralTab {
            id: viewEntryGeneralTab
        },
        ViewEntryExtrasTab {
            id: viewEntryExtrasTab
        },
        ViewEntryHistoryTab {
            id: viewEntryHistoryTab
        }
    ]
}
