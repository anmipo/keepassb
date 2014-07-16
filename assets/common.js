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
