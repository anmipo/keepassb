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
    /**
     * Returns the singleton instance of PasswordGenerator
     */
    static PasswordGenerator* instance();

    /**
     * Returns a random string made of the allowed characters.
     * If all parameters are false, returns an empty string.
     * excludeSimilar ensures the result does not contain any visually similar characters (e.g. Il|1O0)
     */
    Q_INVOKABLE QString makeCustomPassword(const int length, const bool lowerCase, const bool upperCase, const bool numbers,
            const bool specials, const bool excludeSimilar) const;
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
