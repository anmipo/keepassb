/*
 * AppConfig.h
 *
 *  Created on: 31 May 2014
 *      Author: Andrei Popleteev
 */

#ifndef APPCONFIG_H_
#define APPCONFIG_H_

struct AppConfig {
    int clipboardTimeoutSeconds;
};

static const AppConfig appConfig = {
    5 // clipboardTimeoutSeconds
};

#endif /* APPCONFIG_H_ */
