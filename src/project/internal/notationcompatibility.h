#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QVariantMap>
#include <functional>

namespace mu::engraving { class Score; }

namespace mu::project::notationcompatibility {
struct SourceInventory {
    QJsonObject report;
    QJsonArray items;
};

SourceInventory inventoryScore(mu::engraving::Score* score, const QString& sourcePath);
QJsonObject analyzeMei(const QByteArray& mei, const SourceInventory& source,
                      const QHash<QString, QString>& sourceToCanonical = {}, bool canonicalMidiValidated = false);
bool admits(const QJsonObject& report);
QString fatalSummary(const QJsonObject& report);
QByteArray canonicalJson(const QJsonObject& object);

class SummaryModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap report READ report NOTIFY reportChanged)
    Q_PROPERTY(QVariantList findings READ findings NOTIFY reportChanged)
    Q_PROPERTY(QString selectedMode READ selectedMode WRITE setSelectedMode NOTIFY reportChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY reportChanged)
    Q_PROPERTY(QVariantList musicianFindings READ musicianFindings NOTIFY reportChanged)
    Q_PROPERTY(QString musicianTitle READ musicianTitle NOTIFY reportChanged)
    Q_PROPERTY(bool needsFix READ needsFix NOTIFY reportChanged)
public:
    static SummaryModel* instance();
    QVariantMap report() const;
    QVariantList findings() const;
    QString selectedMode() const;
    void setSelectedMode(const QString& mode);
    QString summary() const;
    QVariantList musicianFindings() const;
    QString musicianTitle() const;
    bool needsFix() const;
    bool setReport(const QJsonObject& report, const QHash<QString, int>& measureOrdinals = {},
                   std::function<bool(int, bool)> navigate = {});
    Q_INVOKABLE bool goToProblem(int findingIndex);
signals:
    void reportChanged();
private:
    explicit SummaryModel(QObject* parent);
    QJsonObject m_report;
    QString m_mode = QStringLiteral("game_solo");
    QSet<QString> m_presented;
    QHash<QString, int> m_measureOrdinals;
    std::function<bool(int, bool)> m_navigate;
};
}
