/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "welcomedialogmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::appshell;

static std::vector<QVariantMap> welcomeDialogData()
{
    QVariantMap welcome;
    welcome.insert("title", muse::qtrc("appshell/welcome", "Welcome to Pianomania Composer"));
    welcome.insert("isWelcome", true);
    welcome.insert("description", muse::qtrc("appshell/welcome",
        "Follow this quick tutorial or check out the full Composer guide below on our website."));
    welcome.insert("buttonText", muse::qtrc("appshell/welcome", "Full Composer Guide"));
    welcome.insert("destinationUrl", "https://pianomania.gg/docs/composer");

    QVariantMap account;
    account.insert("title", muse::qtrc("appshell/welcome", "Sign in once"));
    account.insert("imageUrl", "qrc:/resources/welcomedialog/ComposerScore.png");
    account.insert("menuAction", muse::qtrc("appshell/welcome", "Sign in to Pianomania"));
    account.insert("menuHint", muse::qtrc("appshell/welcome", "Use your game account. Composer remembers you."));
    account.insert("description", muse::qtrc("appshell/welcome",
        "Open a piano score, then choose File > Sign in to Pianomania. Use the same account as the game. Composer remembers your sign-in. Masterworks is required to export."));
    account.insert("buttonText", muse::qtrc("appshell/welcome", "Full Composer Guide"));
    account.insert("destinationUrl", "https://pianomania.gg/docs/composer");

    QVariantMap exportSong;
    exportSong.insert("title", muse::qtrc("appshell/welcome", "Export your song"));
    exportSong.insert("imageUrl", "qrc:/resources/welcomedialog/ExportSong.png");
    exportSong.insert("menuAction", muse::qtrc("appshell/welcome", "Export Pianomania file (.pm)"));
    exportSong.insert("menuHint", muse::qtrc("appshell/welcome", "Find it below your Pianomania account in the File menu."));
    exportSong.insert("description", muse::qtrc("appshell/welcome",
        "Choose File > Export Pianomania file (.pm), just below your Pianomania account. Choose a PNG or JPEG cover image, then save your .pm file."));
    exportSong.insert("buttonText", muse::qtrc("appshell/welcome", "Full Composer Guide"));
    exportSong.insert("destinationUrl", "https://pianomania.gg/docs/composer");

    QVariantMap play;
    play.insert("title", muse::qtrc("appshell/welcome", "Import into Pianomania"));
    play.insert("imageUrl", "qrc:/resources/welcomedialog/ImportSong.png");
    play.insert("description", muse::qtrc("appshell/welcome",
        "Move the .pm file to the device you play on. In Pianomania, open Profile > Composer > Import .pm file. Use the same account, then find your song in Song Select."));
    play.insert("buttonText", muse::qtrc("appshell/welcome", "Full Composer Guide"));
    play.insert("destinationUrl", "https://pianomania.gg/docs/composer");

    return { welcome, account, exportSong, play };
}

WelcomeDialogModel::WelcomeDialogModel()
    : muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void WelcomeDialogModel::init()
{
    IF_ASSERT_FAILED(configuration()) {
        return;
    }

    m_items = welcomeDialogData();

    m_currentIndex = 0;

    emit itemsChanged();
    emit currentItemChanged();
}

QVariantMap WelcomeDialogModel::currentItem() const
{
    if (m_items.empty()) {
        return QVariantMap();
    }
    return m_items.at(m_currentIndex);
}

void WelcomeDialogModel::nextItem()
{
    IF_ASSERT_FAILED(!m_items.empty()) {
        return;
    }

    if (!hasNext()) {
        return;
    }
    ++m_currentIndex;
    emit currentItemChanged();
}

void WelcomeDialogModel::prevItem()
{
    IF_ASSERT_FAILED(!m_items.empty()) {
        return;
    }

    if (!hasPrev()) {
        return;
    }
    --m_currentIndex;
    emit currentItemChanged();
}

bool WelcomeDialogModel::showOnStartup() const
{
    return configuration()->welcomeDialogShowOnStartup();
}

void WelcomeDialogModel::setShowOnStartup(bool show)
{
    if (show == showOnStartup()) {
        return;
    }

    configuration()->setWelcomeDialogShowOnStartup(show);
    emit showOnStartupChanged();
}
