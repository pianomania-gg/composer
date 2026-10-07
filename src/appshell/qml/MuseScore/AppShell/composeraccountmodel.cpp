// SPDX-License-Identifier: GPL-3.0-only
#include "composeraccountmodel.h"
#include <QApplication>

namespace mu::appshell {
ComposerAccountModel::ComposerAccountModel(QObject* parent) : QObject(parent) {
    connect(project::composer::accountState(), &project::composer::AccountState::changed, this, &ComposerAccountModel::changed);
}
void ComposerAccountModel::openAccount() { project::composer::showAccount(QApplication::activeWindow()); }
}
