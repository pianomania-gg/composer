// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QByteArray>
#include <QString>
#include <QJsonObject>
#include <QObject>
class QWidget;

namespace mu::project::composer {
class AccountState : public QObject {
    Q_OBJECT
public:
    explicit AccountState(QObject* parent = nullptr);
    bool signedIn() const { return !m_uid.isEmpty(); }
    QString email() const { return m_email; }
    QString error() const { return m_error; }
    void refresh();
signals:
    void changed();
private:
    QString m_uid;
    QString m_email;
    QString m_error;
};
AccountState* accountState();
void showAccount(QWidget* parent);
QByteArray license(const QString& hash, QWidget* parent);
QString currentUid();
QJsonObject sessionFromFirebaseResponses(const QJsonObject& exchange, const QJsonObject& account,
                                        const QString& uid, const QString& apiKey);
}
