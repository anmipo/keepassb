/*
 * PasswordGenerator.h
 *
 *  Created on: 21 Sep 2014
 *      Author: Andrei Popleteev
 */

#ifndef PASSWORDGENERATOR_H_
#define PASSWORDGENERATOR_H_

#include <QObject>

class PasswordGenerator: public QObject {
    Q_OBJECT
private:
    static PasswordGenerator* _instance;

    PasswordGenerator(QObject* parent = 0);
public:
    enum PwGenPreset {
        PWGEN_PRESET_DEFAULT = 0x00,
        PWGEN_PRESET_CUSTOM  = 0x01,
        PWGEN_PRESET_HEX40   = 0x02,
        PWGEN_PRESET_HEX128  = 0x03,
        PWGEN_PRESET_HEX256  = 0x04,
        PWGEN_PRESET_MAC_ADDRESS = 0x05
    };
    Q_ENUMS(PwGenPreset);

    // Password generator flags can be combined with OR
    enum PwGenFlags {
        PWGEN_INCLUDE_LOWER    = 0x01,
        PWGEN_INCLUDE_UPPER    = 0x02,
        PWGEN_INCLUDE_DIGITS   = 0x04,
        PWGEN_INCLUDE_SPECIALS = 0x08,
        PWGEN_EXCLUDE_SIMILAR  = 0x80
    };
    Q_ENUMS(PwGenFlags);

    /**
     * Returns the singleton instance of PasswordGenerator
     */
    static PasswordGenerator* instance();

    /**
     * Returns a random string made of the allowed characters.
     * Flags define included and excluded character sets (see Settings::PWGEN_INCLUDE_* and Setting::PWGEN_EXCLUDE_*)
     * If no charset included, returns an empty string.
     */
    Q_INVOKABLE QString makeCustomPassword(const int length, const int flags) const;
    /**
     * Returns a string containing nBytes of random data in hex representation.
     */
    Q_INVOKABLE QString makeHexPassword(const int nBytes) const;
    /**
     * Returns a random MAC address (format: HH-HH-HH-HH-HH-HH)
     */
    Q_INVOKABLE QString makeMacAddress() const;
};

#endif /* PASSWORDGENERATOR_H_ */
