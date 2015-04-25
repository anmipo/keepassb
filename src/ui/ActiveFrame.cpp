/*
 * ActiveFrame.cpp
 *
 *  Created on: 21 Jun 2014
 *      Author: Andrei
 */

#include "ui/ActiveFrame.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Container>

using namespace bb::cascades;

ActiveFrame::ActiveFrame(QObject* parent, PwDatabaseFacade* database) : SceneCover(parent) {

    QmlDocument* qml = QmlDocument::create("qrc:/assets/ActiveFrameCover.qml").parent(this);
    qml->setContextProperty("database", database);

    Container* mainContainer = qml->createRootObject<Container>();
    setContent(mainContainer);
}

