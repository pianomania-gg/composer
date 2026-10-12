// SPDX-License-Identifier: GPL-3.0-only
#include "composersongdetails.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <functional>
#include <array>
#include <cmath>

static void initSongDetailsResources()
{
    Q_INIT_RESOURCE(composer_song_details);
}

namespace mu::project::composer {
namespace {
struct SongDetailsFonts {
    QString controls;
    QString title;
    QString rating;
};

const SongDetailsFonts& songDetailsFonts()
{
    static const SongDetailsFonts fonts = [] {
        initSongDetailsResources();
        auto load = [](const QString& file) {
            const int id = QFontDatabase::addApplicationFont(":/composer/song-details/" + file);
            return QFontDatabase::applicationFontFamilies(id).value(0);
        };
        return SongDetailsFonts { load("Inter-Regular.ttf"), load("Inter-Medium.ttf"),
                                  load("BarlowCondensed-ExtraBoldItalic.ttf") };
    }();
    return fonts;
}

// The outer dialog stays rectangular; internal surfaces share the game's 7° angle.
// Same circular tangent arcs and uniform radius clamp as the game's SkewGeometry.
// Game action/card radii (16/18) are halved for Composer's smaller control scale.
QPainterPath skewSurface(const QRectF& rect, qreal requestedRadius)
{
    const qreal inset = rect.height() * 0.12278456; // tan(7°)
    const std::array<QPointF, 4> corners { QPointF(rect.left() + inset, rect.top()), rect.topRight(),
                                         QPointF(rect.right() - inset, rect.bottom()), rect.bottomLeft() };
    auto length = [](const QPointF& point) { return std::hypot(point.x(), point.y()); };
    std::array<QPointF, 4> previous, next;
    std::array<qreal, 4> halfAngles, tangents;
    qreal radius = qMin(requestedRadius, rect.height() * 0.5);
    for (int i = 0; i < 4; ++i) {
        previous[i] = corners[(i + 3) % 4] - corners[i];
        next[i] = corners[(i + 1) % 4] - corners[i];
        previous[i] /= length(previous[i]);
        next[i] /= length(next[i]);
        const qreal dot = QPointF::dotProduct(previous[i], next[i]);
        halfAngles[i] = std::acos(qBound(qreal(-1), dot, qreal(1))) * 0.5;
        tangents[i] = radius / std::tan(halfAngles[i]);
    }
    qreal factor = 1;
    for (int i = 0; i < 4; ++i) {
        factor = qMin(factor, length(corners[(i + 1) % 4] - corners[i]) / (tangents[i] + tangents[(i + 1) % 4]));
    }
    radius *= factor;
    QPainterPath path;
    constexpr qreal radiansToDegrees = 180.0 / 3.14159265358979323846;
    for (int i = 0; i < 4; ++i) {
        const qreal tangent = radius / std::tan(halfAngles[i]);
        const QPointF entry = corners[i] + previous[i] * tangent;
        QPointF bisector = previous[i] + next[i];
        bisector /= length(bisector);
        const QPointF center = corners[i] + bisector * (radius / std::sin(halfAngles[i]));
        if (i == 0) path.moveTo(entry); else path.lineTo(entry);
        const qreal start = -std::atan2(entry.y() - center.y(), entry.x() - center.x()) * radiansToDegrees;
        const qreal sweep = -(180.0 - 2.0 * halfAngles[i] * radiansToDegrees);
        path.arcTo(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2), start, sweep);
    }
    path.closeSubpath();
    return path;
}

class SongDetailsButton : public QPushButton {
public:
    SongDetailsButton(const QString& text, bool primary, QWidget* parent)
        : QPushButton(text, parent), m_primary(primary)
    {
        setMinimumSize(primary ? 126 : 104, 42);
        setCursor(Qt::PointingHandCursor);
    }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF face(1, 1, width() - 2, height() - 5);
        painter.fillPath(skewSurface(face.translated(0, 3), 8), QColor("#02070d"));
        QLinearGradient fill(face.topLeft(), face.bottomLeft());
        fill.setColorAt(0, QColor(m_primary ? (underMouse() ? "#3d9256" : "#347c4a") : "#273443"));
        fill.setColorAt(1, QColor(m_primary ? (isDown() ? "#205b35" : "#29643c") : "#17212d"));
        painter.setPen(QPen(QColor(hasFocus() ? "#ecf2f8" : m_primary ? "#65a878" : "#536377"), hasFocus() ? 2 : 1));
        painter.setBrush(fill);
        painter.drawPath(skewSurface(face, 8));
        QFont font(songDetailsFonts().rating);
        font.setPixelSize(24);
        font.setItalic(true);
        font.setWeight(QFont::ExtraBold);
        painter.setFont(font);
        painter.setPen(QColor("#f1f4f8"));
        painter.drawText(face, Qt::AlignCenter, text().toUpper());
    }
private:
    bool m_primary;
};

class SongDetailsCard : public QFrame {
public:
    explicit SongDetailsCard(QWidget* parent) : QFrame(parent) {}
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF face(1, 1, width() - 2, height() - 3);
        QLinearGradient fill(face.topLeft(), face.bottomLeft());
        fill.setColorAt(0, QColor("#1b2735"));
        fill.setColorAt(1, QColor("#0b131e"));
        painter.setBrush(fill);
        painter.setPen(QPen(QColor("#3a4e68"), 1));
        painter.drawPath(skewSurface(face, 9));
    }
};
}

std::optional<SongDetails> chooseSongDetails(QWidget* parent, const SongDetails& details)
{
    QDialog dialog(parent);
    dialog.setObjectName("composerSongDetailsDialog");
    dialog.setWindowTitle(QDialog::tr("Song details"));
    dialog.resize(800, 520);
    dialog.setMinimumWidth(640);
    const auto& fonts = songDetailsFonts();
    dialog.setFont(QFont(fonts.controls));
    dialog.setStyleSheet(QString(
        "QDialog#composerSongDetailsDialog { background: #050c14; color: #edf2f8; border: 1px solid #3a4e68; }"
        "QLabel { color: #c7d3e3; font-size: 14px; background: transparent; }"
        "QLabel#songDetailsHeading { color: #f1f4f8; font-size: 26px; font-weight: 600; }"
        "QLabel#songCardTitle { color: #f1f4f8; font-family: '%1'; font-size: 26px; }"
        "QLabel#songCardDr { color: #e0a33f; font-family: '%2'; font-size: 30px; font-weight: 800; font-style: italic; }"
        "QLabel#songDetailsError { color: #ffaca4; }"
        "QLineEdit, QComboBox, QSpinBox { color: #edf2f8; padding: 9px 10px; border: 1px solid #34475f; border-radius: 2px; background: #02070d; font-size: 14px; selection-background-color: #32567a; selection-color: #ffffff; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #8faaca; }"
        "QComboBox { padding-right: 24px; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox::down-arrow { image: url(:/composer/song-details/arrow-down.svg); width: 12px; height: 8px; }"
        "QComboBox QAbstractItemView { background: #17212d; color: #edf2f8; selection-background-color: #32567a; selection-color: #ffffff; border: 1px solid #536377; padding: 4px; }"
        "QSpinBox#songDetailsDr { color: #e0a33f; font-family: '%2'; font-size: 24px; font-weight: 800; font-style: italic; padding-top: 4px; padding-bottom: 4px; }"
        "QCheckBox { color: #c7d3e3; font-size: 13px; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #65768b; border-radius: 2px; background: #02070d; }"
        "QCheckBox::indicator:checked { background: #c58c31; border-color: #c58c31; image: url(:/composer/song-details/check.svg); }"
        "QCheckBox:focus { color: #ffffff; }"
        "QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; border: none; }"
        "QScrollBar:vertical { background: #0d1723; width: 6px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #536377; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        "QSlider::groove:horizontal { height: 8px; background: #02070d; border: 1px solid #34475f; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #805f2e; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 18px; margin: -6px 0; border: 1px solid #8796aa; border-radius: 2px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #253141,stop:0.45 #8191a5,stop:0.55 #c6d1de,stop:1 #253141); }"
        "QSlider::handle:horizontal:focus { border-color: #edf2f8; }"
        "QPushButton#addSongCredit { color: #edf2f8; background: transparent; border: none; border-radius: 4px; padding: 0; font-size: 24px; }"
        "QPushButton#addSongCredit:hover, QPushButton#addSongCredit:focus { color: #ffffff; background: #26364b; }"
        "QToolButton { color: #9aabc0; background: transparent; border: none; padding: 6px; font-size: 20px; }"
        "QToolButton:hover, QToolButton:focus { color: #ffffff; background: #26364b; }"
        "QFrame#songDetailsRule { background: #283648; max-height: 1px; border: none; }"
    ).arg(fonts.title, fonts.rating));
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 20);
    layout->setSpacing(16);
    auto* header = new QHBoxLayout;
    auto* heading = new QLabel(QDialog::tr("Song details"), &dialog);
    heading->setObjectName("songDetailsHeading");
    header->addWidget(heading);
    header->addStretch();
    layout->addLayout(header);
    auto addRule = [&] {
        auto* rule = new QFrame(&dialog);
        rule->setObjectName("songDetailsRule");
        rule->setFixedHeight(1);
        layout->addWidget(rule);
    };
    addRule();
    auto* content = new QHBoxLayout;
    content->setSpacing(28);
    layout->addLayout(content, 1);
    auto* form = new QVBoxLayout;
    form->setSpacing(8);
    content->addLayout(form, 1);
    auto* titleLabel = new QLabel(QDialog::tr("Song title"), &dialog);
    auto* title = new QLineEdit(details.title, &dialog);
    title->setObjectName("songDetailsTitle");
    title->setMaxLength(200);
    title->setAccessibleName(titleLabel->text());
    titleLabel->setBuddy(title);
    form->addWidget(titleLabel);
    form->addWidget(title);
    form->addSpacing(12);
    auto* creditHeader = new QHBoxLayout;
    creditHeader->setSpacing(8);
    creditHeader->addWidget(new QLabel(QDialog::tr("Composers & arrangers"), &dialog));
    auto* add = new QPushButton("+", &dialog);
    add->setObjectName("addSongCredit");
    add->setFixedSize(30, 30);
    add->setAutoDefault(false);
    add->setCursor(Qt::PointingHandCursor);
    add->setAccessibleName(QDialog::tr("Add composer or arranger"));
    add->setToolTip(QDialog::tr("Add composer or arranger"));
    creditHeader->addWidget(add);
    creditHeader->addStretch();
    form->addLayout(creditHeader);

    auto* scroll = new QScrollArea(&dialog);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setMinimumHeight(0);
    auto* creditContainer = new QWidget(scroll);
    auto* creditLayout = new QVBoxLayout(creditContainer);
    creditLayout->setContentsMargins(0, 0, 0, 0);
    creditLayout->setSpacing(8);
    creditLayout->addStretch();
    scroll->setWidget(creditContainer);
    form->addWidget(scroll, 1);
    form->addSpacing(10);
    auto* drHeader = new QHBoxLayout;
    drHeader->addWidget(new QLabel(QDialog::tr("Difficulty rating"), &dialog));
    drHeader->addStretch();
    auto* enabled = new QCheckBox(QDialog::tr("Set DR"), &dialog);
    enabled->setObjectName("songDetailsDrEnabled");
    enabled->setChecked(details.difficultyRating.has_value());
    drHeader->addWidget(enabled);
    form->addLayout(drHeader);
    auto* drControls = new QWidget(&dialog);
    auto* drLayout = new QHBoxLayout(drControls);
    drLayout->setContentsMargins(0, 0, 0, 0);
    auto* number = new QSpinBox(drControls);
    number->setObjectName("songDetailsDr");
    number->setAccessibleName(QDialog::tr("Difficulty rating"));
    number->setRange(0, 999);
    number->setValue(details.difficultyRating.value_or(0));
    number->setPrefix("DR  ");
    number->setFixedWidth(112);
    auto* slider = new QSlider(Qt::Horizontal, drControls);
    slider->setObjectName("songDetailsDrSlider");
    slider->setAccessibleName(QDialog::tr("Adjust difficulty rating"));
    slider->setRange(0, 999);
    slider->setValue(number->value());
    drLayout->addWidget(number);
    drLayout->addWidget(slider, 1);
    form->addWidget(drControls);
    form->addStretch();
    QObject::connect(number, &QSpinBox::valueChanged, slider, &QSlider::setValue);
    QObject::connect(slider, &QSlider::valueChanged, number, &QSpinBox::setValue);

    auto* preview = new QVBoxLayout;
    content->addLayout(preview);
    auto* card = new SongDetailsCard(&dialog);
    card->setObjectName("songCard");
    card->setFixedWidth(248);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(44, 24, 28, 24);
    cardLayout->setSpacing(10);
    auto* cardTitle = new QLabel(card);
    cardTitle->setObjectName("songCardTitle");
    auto* composers = new QLabel(card);
    composers->setObjectName("songCardComposers");
    auto* arrangers = new QLabel(card);
    arrangers->setObjectName("songCardArrangers");
    auto* cardDr = new QLabel(card);
    cardDr->setObjectName("songCardDr");
    for (auto* label : { cardTitle, composers, arrangers, cardDr }) {
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        cardLayout->addWidget(label);
    }
    preview->addWidget(card);
    preview->addStretch();

    auto* error = new QLabel(&dialog);
    error->setObjectName("songDetailsError");
    error->setTextFormat(Qt::PlainText);
    error->setWordWrap(true);
    error->hide();
    layout->addWidget(error);
    addRule();
    auto* buttons = new QDialogButtonBox(&dialog);
    auto* cancel = new SongDetailsButton(QDialog::tr("Cancel"), false, &dialog);
    buttons->addButton(cancel, QDialogButtonBox::RejectRole);
    auto* apply = new SongDetailsButton(QDialog::tr("Apply"), true, &dialog);
    buttons->addButton(apply, QDialogButtonBox::AcceptRole);
    apply->setObjectName("applySongDetails");
    apply->setDefault(true);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    struct Row { QWidget* widget; QComboBox* role; QLineEdit* name; };
    QList<Row> rows;
    auto collect = [&] {
        SongDetails result;
        result.title = title->text().trimmed();
        for (const auto& row : rows) result.credits.append({row.role->currentData().toString(), row.name->text().trimmed()});
        if (enabled->isChecked()) result.difficultyRating = number->value();
        return result;
    };
    auto update = [&] {
        scroll->setFixedHeight(qMin(220, rows.size() * 48));
        const auto result = collect();
        cardTitle->setText(result.title);
        QStringList composerNames, arrangerNames;
        for (const auto& credit : result.credits) {
            if (!credit.name.isEmpty()) (credit.role == "composer" ? composerNames : arrangerNames).append(credit.name);
        }
        composers->setText(composerNames.join(" · "));
        composers->setVisible(!composerNames.isEmpty());
        arrangers->setText(QDialog::tr("Arranged by %1").arg(arrangerNames.join(" · ")));
        arrangers->setVisible(!arrangerNames.isEmpty());
        cardDr->setText(QString("DR %1").arg(number->value()));
        cardDr->setVisible(enabled->isChecked());
        drControls->setVisible(enabled->isChecked());
        error->hide();
    };
    auto addRow = [&](const SongCredit& credit, bool focus) {
        auto* widget = new QWidget(creditContainer);
        auto* row = new QHBoxLayout(widget);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(8);
        auto* role = new QComboBox(widget);
        role->setObjectName("songCreditRole");
        role->setAccessibleName(QDialog::tr("Composer or arranger"));
        role->addItem(QDialog::tr("Composer"), "composer");
        role->addItem(QDialog::tr("Arranger"), "arranger");
        role->setCurrentIndex(credit.role == "arranger" ? 1 : 0);
        role->setFixedWidth(140);
        auto* name = new QLineEdit(credit.name, widget);
        name->setObjectName("songCreditName");
        name->setAccessibleName(QDialog::tr("Name"));
        name->setPlaceholderText(QDialog::tr("Name"));
        name->setMaxLength(160);
        name->setCursorPosition(0);
        auto* remove = new QToolButton(widget);
        remove->setObjectName("removeSongCredit");
        remove->setText("×");
        remove->setAccessibleName(QDialog::tr("Remove name"));
        remove->setToolTip(QDialog::tr("Remove name"));
        row->addWidget(role); row->addWidget(name, 1); row->addWidget(remove);
        rows.append({widget, role, name});
        creditLayout->insertWidget(creditLayout->count() - 1, widget);
        QObject::connect(role, &QComboBox::currentIndexChanged, &dialog, update);
        QObject::connect(name, &QLineEdit::textChanged, &dialog, update);
        QObject::connect(remove, &QToolButton::clicked, &dialog, [&, widget] {
            for (int i = 0; i < rows.size(); ++i) if (rows[i].widget == widget) { rows.removeAt(i); break; }
            creditLayout->removeWidget(widget);
            widget->hide(); widget->deleteLater(); update(); add->setFocus();
        });
        update();
        if (focus) name->setFocus();
    };
    for (const auto& credit : details.credits) addRow(credit, false);
    QObject::connect(add, &QPushButton::clicked, &dialog, [&] { addRow({"composer", {}}, true); });
    QObject::connect(title, &QLineEdit::textChanged, &dialog, update);
    QObject::connect(number, &QSpinBox::valueChanged, &dialog, update);
    QObject::connect(enabled, &QCheckBox::toggled, &dialog, update);
    QObject::connect(apply, &QPushButton::clicked, &dialog, [&] {
        const auto result = collect();
        if (result.title.isEmpty()) {
            error->setText(QDialog::tr("Enter a song title.")); error->show(); title->setFocus(); return;
        }
        for (const auto& row : rows) if (row.name->text().trimmed().isEmpty()) {
            error->setText(QDialog::tr("Enter a name or remove the empty row."));
            error->show(); row.name->setFocus(); return;
        }
        dialog.accept();
    });
    update();
    title->setFocus();
    if (dialog.exec() != QDialog::Accepted) return std::nullopt;
    return collect();
}
}
