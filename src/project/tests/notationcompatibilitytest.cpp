#include <gtest/gtest.h>
#include <QJsonArray>
#include "project/internal/notationcompatibility.h"

using namespace mu::project::notationcompatibility;
namespace {
QByteArray document(const QByteArray& content)
{
    return "<mei><music><body><mdiv><score><section><measure xml:id='m1' n='1'><staff n='1'><layer n='1'>"
        + content + "</layer></staff></measure></section></score></mdiv></body></music></mei>";
}
const QByteArray note = "<note xml:id='n1' idx='0' pname='c' oct='4' dur='4' beat='1'/>";
}
TEST(NotationCompatibility, WarningRetainsLocationsAndUnverifiedMidiAdmission)
{
    auto report = analyzeMei(document(note + "<arpeg xml:id='a1' startid='#n1'/>"), inventoryScore(nullptr, "fixture"));
    EXPECT_TRUE(admits(report));
    auto findings = report["findings"].toArray(); ASSERT_EQ(findings.size(), 1);
    auto row = findings.first().toObject(); EXPECT_EQ(row["count"].toInt(), 1);
    EXPECT_EQ(row["countUnit"].toString(), "mei_item");
    EXPECT_EQ(row["locations"].toArray().first().toObject()["anchorIds"].toArray().first().toString(), "n1");
    EXPECT_EQ(report["completeness"].toObject()["practice"].toObject()["traversal"].toString(), "unverified");
    const QByteArray segment = "<pm:gliss-segment page='0' start='0,0' end='1,1' width='.01'>"
        "<pm:text font='Edwin' font-style='2' corners='0,0;1,0;1,1;0,1'>gliss.</pm:text></pm:gliss-segment>";
    const QByteArray gliss = "<gliss xmlns:pm='http://pianomania.gg/ns/mei' xml:id='g1' startid='#n1' endid='#n2'"
        " lform='solid' visible='true' pm:gliss-play='true' pm:gliss-start-center='0,0' pm:gliss-end-center='1,1'>"
        + segment + "</gliss>";
    const QByteArray notes = note + QByteArray(note).replace("n1", "n2");
    const QByteArray supportedMei = document(notes + gliss);
    auto supported = analyzeMei(supportedMei, inventoryScore(nullptr, "fixture"));
    ASSERT_TRUE(admits(supported));
    auto supportedRows = supported["findings"].toArray(); ASSERT_EQ(supportedRows.size(), 1);
    EXPECT_EQ(supportedRows.first().toObject()["reasonCode"].toString(), "glissando_render_unverified");
    EXPECT_TRUE(supportedRows.first().toObject()["omittedBehavior"].toArray().isEmpty());
    auto unsupported = analyzeMei(document(notes + QByteArray(gliss).replace("Edwin", "Arial")), inventoryScore(nullptr, "fixture"));
    auto unsupportedRows = unsupported["findings"].toArray(); ASSERT_EQ(unsupportedRows.size(), 1);
    EXPECT_EQ(unsupportedRows.first().toObject()["reasonCode"].toString(), "glissando_text_omission");
    EXPECT_EQ(unsupportedRows.first().toObject()["observedBehavior"].toString(), "partial");
    auto multiple = analyzeMei(document(notes + QByteArray(gliss).replace(segment, segment + segment)), inventoryScore(nullptr, "fixture"));
    EXPECT_EQ(multiple["findings"].toArray().first().toObject()["reasonCode"].toString(), "glissando_multisegment_omission");
    auto legacy = analyzeMei(document(notes + QByteArray(gliss).replace(segment, "")), inventoryScore(nullptr, "fixture"));
    EXPECT_EQ(legacy["findings"].toArray().first().toObject()["reasonCode"].toString(), "glissando_geometry_missing");
    EXPECT_EQ(supportedMei, document(notes + gliss));
}
TEST(NotationCompatibility, UnknownWrapperRejectsOncePerWrapper)
{
    auto report = analyzeMei(document("<futureWrapper>" + note + QByteArray(note).replace("n1", "n2") + "</futureWrapper>"), inventoryScore(nullptr, "fixture"));
    EXPECT_FALSE(admits(report));
    for (const auto& value : report["findings"].toArray()) {
        auto row = value.toObject();
        if (row["reasonCode"] == "unsupported_playable_wrapper") EXPECT_EQ(row["count"].toInt(), 1);
    }
    for (const QByteArray& wrapper : {QByteArray("bTrem"), QByteArray("fTrem")}) {
        QByteArray mei = document("<" + wrapper + ">" + note + "</" + wrapper + ">");
        EXPECT_FALSE(admits(analyzeMei(mei, inventoryScore(nullptr, "fixture"))));
        auto validated = analyzeMei(mei, inventoryScore(nullptr, "fixture"), {}, true);
        EXPECT_TRUE(admits(validated));
        auto rows = validated["findings"].toArray(); ASSERT_EQ(rows.size(), 1);
        EXPECT_EQ(rows.first().toObject()["observedBehavior"].toString(), "unverified");
    }
}
TEST(NotationCompatibility, ExistingComponentGroupsHaveNoFalseArticulationWarning)
{
    for (const QByteArray& tokens : {QByteArray("acc stacc"), QByteArray("stacc acc"), QByteArray("ten marc"), QByteArray("marc ten")}) {
        QByteArray mei = document(note + "<artic artic='" + tokens + "'/>");
        QByteArray original = mei;
        auto report = analyzeMei(mei, inventoryScore(nullptr, "fixture"));
        EXPECT_TRUE(admits(report));
        EXPECT_TRUE(report["findings"].toArray().isEmpty());
        EXPECT_EQ(mei, original);
        EXPECT_EQ(report["completeness"].toObject()["practice"].toObject()["scoring"].toString(), "unverified");
    }
}
TEST(NotationCompatibility, UnknownComponentsKeepSpecificDiagnostics)
{
    for (const QByteArray& tokens : {QByteArray("spicc"), QByteArray("future-artic"), QByteArray("stacc future-artic")}) {
        auto report = analyzeMei(document(note + "<artic artic='" + tokens + "'/>"), inventoryScore(nullptr, "fixture"));
        EXPECT_TRUE(admits(report));
        auto findings = report["findings"].toArray(); ASSERT_EQ(findings.size(), 1);
        auto row = findings.first().toObject(); EXPECT_EQ(row["count"].toInt(), 1);
        EXPECT_EQ(row["variant"].toString().toUtf8(), tokens);
        EXPECT_EQ(row["reasonCode"].toString(), tokens.contains(' ') ? "unsupported_articulation_combination" : "unsupported_articulation");
    }
}
TEST(NotationCompatibility, MissingIdentityRejectsAndCanonicalBytesStayUnchanged)
{
    QByteArray mei = document("<note idx='0' pname='c' oct='4' dur='4' beat='1'/>");
    QByteArray original = mei;
    auto report = analyzeMei(mei, inventoryScore(nullptr, "fixture"));
    EXPECT_FALSE(admits(report)); EXPECT_EQ(mei, original);
}
