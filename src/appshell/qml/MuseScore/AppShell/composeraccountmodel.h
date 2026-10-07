// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QObject>
#include <qqmlintegration.h>
#include "project/internal/composersession.h"

namespace mu::appshell {
class ComposerAccountModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool signedIn READ signedIn NOTIFY changed)
    Q_PROPERTY(QString email READ email NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(bool developmentBuild READ developmentBuild CONSTANT)
public:
    explicit ComposerAccountModel(QObject* parent = nullptr);
    bool signedIn() const { return project::composer::accountState()->signedIn(); }
    QString email() const { return project::composer::accountState()->email(); }
    QString error() const { return project::composer::accountState()->error(); }
    bool developmentBuild() const {
#ifdef PIANOMANIA_COMPOSER_PRODUCTION
        return false;
#else
        return true;
#endif
    }
    Q_INVOKABLE void openAccount();
signals:
    void changed();
};
}
