/*
 * ActiveFrame.h
 *
 * Active frame for the minimized state of the app.
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
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
