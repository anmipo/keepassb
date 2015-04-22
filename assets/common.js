/**
 * Common functions used by different QML files
 */

/**
 * Returns list item description string for the given entry,
 * according to app settings.
 */
function getEntryDescription(entry) {
    switch (appSettings.entryListDetail) {
        case 0: // None
            return "";
        case 1: // User name
            return entry.userName;
        case 2: // Password
            return entry.password;
        case 4: // URL
            return entry.url;
        case 8: // Notes
            // Multiline notes show up strangely, so we make them one-liners
            var note = entry.notes.replace(/[\r\n]/g,' ');
            return note;
        case 16: // Last modified time
            return entry.lastModificationTime.toString();
        default:
            return "";
    }
}

/**
 * Returns localized text describing the given quick unlock type.
 * @param quickUnlockType
 */
function getQuickUnlockTypeDescription(quickUnlockType) {
    switch (quickUnlockType) {
    case Settings.QUICK_UNLOCK_FIRST_3:
        return qsTr("First 3 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    First 3 symbols of the password'");
    case Settings.QUICK_UNLOCK_FIRST_4:
        return qsTr("First 4 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    First 4 symbols of the password'");
    case Settings.QUICK_UNLOCK_FIRST_5:
    	return qsTr("First 5 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    First 5 symbols of the password'");
    case Settings.QUICK_UNLOCK_LAST_3:
    	return qsTr("Last 3 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    Last 3 symbols of the password'");
    case Settings.QUICK_UNLOCK_LAST_4:
    	return qsTr("Last 4 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    Last 4 symbols of the password'");
    case Settings.QUICK_UNLOCK_LAST_5:
    	return qsTr("Last 5 symbols of the password", "One of the possible values of the Quick Password setting. Will be displayed as 'Quick Password    Last 5 symbols of the password'");
    }
}
/**
 * Returns the required length of the quick password.
 * @param quickUnlockType
 */
function getQuickPasswordLength(quickUnlockType) {
    switch (quickUnlockType) {
    case Settings.QUICK_UNLOCK_FIRST_3: // fallthrough
    case Settings.QUICK_UNLOCK_LAST_3:
    	return 3;
    case Settings.QUICK_UNLOCK_LAST_4:  // fallthrough
    case Settings.QUICK_UNLOCK_FIRST_4:
    	return 4;
    case Settings.QUICK_UNLOCK_FIRST_5:  // fallthrough
    case Settings.QUICK_UNLOCK_LAST_5:
    	return 5;
    default:
    	return 1000; // an arbitrary but long enough value, just in case
    }
}

/**
 * Returns a nice human-readable file path.
 * Basically, it just cuts off the prefix, such as
 *   "/accounts/1000/shared"
 *   "/accounts/1000/removable"
 *   "/accounts/1000-enterprise/shared"
 *   "/accounts/1000-enterprise/removable"   
 */
function prettifyFilePath(path) {
    return path.split('/').slice(4).join('/');
}

/**
 * Returns a text representation of QDateTime.
 * 
 * @param timestamp
 */
function timestampToString(timestamp) {
	return Qt.formatDateTime(timestamp, Qt.SystemLocaleLongDate);
}

/**
 * Deletes the given entry (with backup) and saves the database.
 * @param entry
 */
function deleteEntry(entry) {
    Qt.app.restartWatchdog();
    entry.moveToBackup(); // also updates timestamps 
    Qt.database.save();
}

/**
 * Deletes the given group (with backup) and saves the database.
 * @param group
 */
function deleteGroup(group) {
    Qt.app.restartWatchdog();
    group.moveToBackup(); // also updates timestamps
    Qt.database.save();
}

/**
 * Creates an Option of the given DropDown; 
 * deletes the previous option with the same value, if any.
 */
function createUniqueOption(fullPath, dropdown, newOptionComponent) {
    // First, remove any already existing options with this path
    var existingIndex = -1;
    var options = dropdown.options;
    for (var i = 0; i < dropdown.count(); i++) {
        if (options[i].value == fullPath) {
            existingIndex = i;
            break;
        } 
    }
    if (existingIndex >= 0)
        dropdown.remove(dropdown.options[existingIndex]);

    var option = newOptionComponent.createObject();
    option.value = fullPath;
    option.text = prettifyFilePath(fullPath);
    return option;
}

/**
 * Performs Multi-Copy function, optionally showing info dialog first.
 */
function performMultiCopy(entry, suppressInfo) {
	if (appSettings.multiCopyFirstUse && !suppressInfo) {
		multiCopyInfoDialog.entry = entry;
		multiCopyInfoDialog.show();
	} else {
		Qt.app.prepareMultiCopy(entry.userName, entry.password);		
	}	
}