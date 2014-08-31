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
	return timestamp.toString();
}