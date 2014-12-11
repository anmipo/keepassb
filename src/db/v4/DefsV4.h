/*
 * DefsV4.h
 *
 *  Created on: 11 Dec 2014
 *      Author: Andrei Popleteev
 */

#ifndef DEFSV4_H_
#define DEFSV4_H_

#include "db/PwDatabase.h"


// KeePass2 XML tag names
const QString XML_META = QString("Meta");
const QString XML_ROOT = QString("Root");
const QString XML_GROUP = QString("Group");
const QString XML_ENTRY = QString("Entry");

const QString XML_GENERATOR                       = QString("Generator");
const QString XML_DATABASE_NAME                   = QString("DatabaseName");
const QString XML_DATABASE_NAME_CHANGED           = QString("DatabaseNameChanged");
const QString XML_DATABASE_DESCRIPTION            = QString("DatabaseDescription");
const QString XML_DATABASE_DESCRIPTION_CHANGED    = QString("DatabaseDescriptionChanged");
const QString XML_DEFAULT_USERNAME                = QString("DefaultUserName");
const QString XML_DEFAULT_USERNAME_CHANGED        = QString("DefaultUserNameChanged");
const QString XML_MAINTENANCE_HISTORY_DAYS        = QString("MaintenanceHistoryDays");
const QString XML_COLOR                           = QString("Color");
const QString XML_MASTER_KEY_CHANGED              = QString("MasterKeyChanged");
const QString XML_MASTER_KEY_CHANGE_REC           = QString("MasterKeyChangeRec");
const QString XML_MASTER_KEY_CHANGE_FORCE         = QString("MasterKeyChangeForce");
const QString XML_MEMORY_PROTECTION               = QString("MemoryProtection");
const QString XML_MEMORY_PROTECTION_PROTECT_TITLE = QString("ProtectTitle");
const QString XML_MEMORY_PROTECTION_PROTECT_USERNAME = QString("ProtectUserName");
const QString XML_MEMORY_PROTECTION_PROTECT_PASSWORD = QString("ProtectPassword");
const QString XML_MEMORY_PROTECTION_PROTECT_URL   = QString("ProtectURL");
const QString XML_MEMORY_PROTECTION_PROTECT_NOTES = QString("ProtectNotes");
const QString XML_RECYCLE_BIN_ENABLED             = QString("RecycleBinEnabled");
const QString XML_RECYCLE_BIN_UUID                = QString("RecycleBinUUID");
const QString XML_RECYCLE_BIN_CHANGED             = QString("RecycleBinChanged");
const QString XML_ENTRY_TEMPLATES_GROUP           = QString("EntryTemplatesGroup");
const QString XML_ENTRY_TEMPLATES_GROUP_CHANGED   = QString("EntryTemplatesGroupChanged");
const QString XML_HISTORY_MAX_ITEMS               = QString("HistoryMaxItems");
const QString XML_HISTORY_MAX_SIZE                = QString("HistoryMaxSize");
const QString XML_LAST_SELECTED_GROUP             = QString("LastSelectedGroup");
const QString XML_LAST_TOP_VISIBLE_GROUP          = QString("LastTopVisibleGroup");
const QString XML_CUSTOM_ICONS                    = QString("CustomIcons");
const QString XML_CUSTOM_ICON_ITEM                = QString("Icon");
const QString XML_CUSTOM_ICON_ITEM_ID             = QString("UUID");
const QString XML_CUSTOM_ICON_ITEM_DATA           = QString("Data");
const QString XML_BINARIES                        = QString("Binaries");
const QString XML_BINARY_ID                       = QString("ID");
const QString XML_BINARY_COMPRESSED               = QString("Compressed");
const QString XML_CUSTOM_DATA                     = QString("CustomData");
const QString XML_CUSTOM_DATA_ITEM                = QString("Item");
const QString XML_BINARY                          = QString("Binary");

const QString XML_TITLE = QString("Title");
const QString XML_USERNAME = QString("UserName");
const QString XML_PASSWORD = QString("Password");
const QString XML_URL = QString("URL");
const QString XML_NOTES = QString("Notes");

const QString XML_UUID = QString("UUID");
const QString XML_NAME = QString("Name");
const QString XML_ICON_ID = QString("IconID");
const QString XML_STRING = QString("String");
const QString XML_HISTORY = QString("History");
const QString XML_KEY = QString("Key");
const QString XML_VALUE = QString("Value");
const QString XML_PROTECTED = QString("Protected");
const QString XML_TRUE = QString("True"); // Since Qt/Cascades does not have string-to-bool conversion, we need this
const QString XML_REF = QString("Ref");

const QString XML_TIMES = QString("Times");
const QString XML_LAST_MODIFICATION_TIME = QString("LastModificationTime");
const QString XML_CREATION_TIME = QString("CreationTime");
const QString XML_LAST_ACCESS_TIME = QString("LastAccessTime");
const QString XML_EXPIRY_TIME = QString("ExpiryTime");
const QString XML_EXPIRES = QString("Expires");

// Tag names for XML-formatted key files
const QString XML_KEYFILE = "KeyFile";
const QString XML_KEYFILE_META = "Meta";
const QString XML_KEYFILE_KEY = "Key";
const QString XML_KEYFILE_DATA = "Data";

/**
 * Common error codes for V4-related methods.
 */
class ErrorCodesV4 {
public:
    enum ErrorCode {
        SUCCESS = PwDatabase::SUCCESS,
        PASSWORD_HASHING_ERROR_1 = 0x10,
        PASSWORD_HASHING_ERROR_2 = 0x11,
        PASSWORD_HASHING_ERROR_3 = 0x12,
        KEY_TRANSFORM_INIT_ERROR = 0x20,
        KEY_TRANSFORM_ERROR_1    = 0x21,
        KEY_TRANSFORM_ERROR_2    = 0x22,
        KEY_TRANSFORM_ERROR_3    = 0x23,
        KEY_TRANSFORM_END_ERROR  = 0x24,
        CANNOT_DECRYPT_DB        = 0x30,
        CANNOT_INIT_SALSA20      = 0x31,
        WRONG_BLOCK_ID           = 0x32,
        BLOCK_HASH_NON_ZERO      = 0x33,
        BLOCK_HASH_MISMATCH      = 0x34,
        XML_PARSING_ERROR        = 0x50,
        XML_META_PARSING_ERROR   = 0x51,
        XML_META_BINARIES_PARSING_ERROR          = 0x52,
        XML_META_MEMORY_PROTECTION_PARSING_ERROR = 0x53,
        XML_META_CUSTOM_DATA_PARSING_ERROR       = 0x54,
        XML_META_CUSTOM_ICONS_PARSING_ERROR      = 0x55,
        XML_META_UNKNOWN_TAG_ERROR               = 0x56,

        XML_TIMES_PARSING_ERROR  = 0x70,
        XML_NO_ROOT_GROUP        = 0x71,
        XML_STRING_VALUE_PARSING_ERROR = 0x72,
        GROUP_LOADING_ERROR            = 0x73,
        ICON_ID_IS_NOT_INTEGER         = 0x74,
        INVALID_ATTACHMENT_REFERENCE   = 0x75,
    };
};


#endif /* DEFSV4_H_ */
