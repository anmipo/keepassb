/*
 * ActiveFrame.cpp
 *
 * Copyright (c) 2014-2016 Andrei Popleteev.
 * Licensed under the MIT license.
 */

#include "ui/ActiveFrame.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Container>

using namespace bb::cascades;

ActiveFrame::ActiveFrame(bb::cascades::Application* appBase, ApplicationUI* app,
        PwDatabaseFacade* database) : SceneCover(appBase) {
    QmlDocument* qml = QmlDocument::create("qrc:/assets/ActiveFrameCover.qml").parent(appBase);
    qml->setContextProperty("app", app);
    qml->setContextProperty("database", database);

    Container* mainContainer = qml->createRootObject<Container>();
    setContent(mainContainer);
}

ActiveFrame::~ActiveFrame() {
    // nothing to do
}
