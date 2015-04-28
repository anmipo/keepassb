/*
 * ActiveFrame.h
 *
 * Active frame for the minimized state of the app.
 *
 *  Created on: 21 Jun 2014
 *      Author: Andrei Popleteev
 */

#ifndef ACTIVEFRAME_H_
#define ACTIVEFRAME_H_

#include <bb/cascades/SceneCover>
#include "db/PwDatabase.h"
#include "applicationui.hpp"

namespace bb {
    namespace cascades {
        class SceneCover;
    }
}

class ActiveFrame : public bb::cascades::SceneCover {
    Q_OBJECT
public:
    ActiveFrame(bb::cascades::Application* appBase, ApplicationUI* app, PwDatabaseFacade* database);
    ~ActiveFrame();
};

#endif /* ACTIVEFRAME_H_ */
