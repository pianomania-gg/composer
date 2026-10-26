#include "notationcompatibility.h"

#include <algorithm>
#include <cmath>
#include <QCoreApplication>
#include <QMap>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/spanner.h"
#include "engraving/types/symnames.h"

using namespace mu::project::notationcompatibility;
namespace {
// BEGIN GENERATED NOTATION CATALOG
static constexpr const char* NOTATION_CATALOG_DIGEST = "87a89fa0e7c3c92e085b50dbc43a0e31eb3b356d5711ab7c2e307d60db99dbe9";
static constexpr const char* NOTATION_CATALOG_JSON =
    R"PMCAT({"acceptedLayerChildren":["note","chord","beam","tuplet","graceGrp","rest","mRest","clef"],"acceptedMeiDurations":["long","breve","1","2","4","8","16","32","64","128","256","512","1024"],"acceptedPianoGlyphCatalog":{"catalogId":"pianomania.piano-engraving-glyphs.v1","schemaVersion":1,"symbols":[{"category":"accidental","codepoint":"E26C","name":"accidentalBracketLeft"},{"category":"accidental","codepoint":"E26D","name":"accidentalBracketRight"},{"category":"accidental","codepoint":"E264","name":"accidentalDoubleFlat"},{"category":"accidental","codepoint":"E263","name":"accidentalDoubleSharp"},{"category":"accidental","codepoint":"E260","name":"accidentalFlat"},{"category":"accidental","codepoint":"E261","name":"accidentalNatural"},{"category":"accidental","codepoint":"E267","name":"accidentalNaturalFlat"},{"category":"accidental","codepoint":"E268","name":"accidentalNaturalSharp"},{"category":"accidental","codepoint":"E26A","name":"accidentalParensLeft"},{"category":"accidental","codepoint":"E26B","name":"accidentalParensRight"},{"category":"accidental","codepoint":"E262","name":"accidentalSharp"},{"category":"accidental","codepoint":"E266","name":"accidentalTripleFlat"},{"category":"accidental","codepoint":"E265","name":"accidentalTripleSharp"},{"category":"arpeggio","codepoint":"EB7C","name":"arrowheadBlackDown"},{"category":"arpeggio","codepoint":"EB78","name":"arrowheadBlackUp"},{"category":"articulation","codepoint":"E4A0","name":"articAccentAbove"},{"category":"articulation","codepoint":"E4A1","name":"articAccentBelow"},{"category":"articulation","codepoint":"E4B0","name":"articAccentStaccatoAbove"},{"category":"articulation","codepoint":"E4B1","name":"articAccentStaccatoBelow"},{"category":"articulation","codepoint":"E4BA","name":"articLaissezVibrerAbove"},{"category":"articulation","codepoint":"E4BB","name":"articLaissezVibrerBelow"},{"category":"articulation","codepoint":"E4AC","name":"articMarcatoAbove"},{"category":"articulation","codepoint":"E4AD","name":"articMarcatoBelow"},{"category":"articulation","codepoint":"E4AE","name":"articMarcatoStaccatoAbove"},{"category":"articulation","codepoint":"E4AF","name":"articMarcatoStaccatoBelow"},{"category":"articulation","codepoint":"E4BC","name":"articMarcatoTenutoAbove"},{"category":"articulation","codepoint":"E4BD","name":"articMarcatoTenutoBelow"},{"category":"articulation","codepoint":"ED40","name":"articSoftAccentAbove"},{"category":"articulation","codepoint":"ED41","name":"articSoftAccentBelow"},{"category":"articulation","codepoint":"ED42","name":"articSoftAccentStaccatoAbove"},{"category":"articulation","codepoint":"ED43","name":"articSoftAccentStaccatoBelow"},{"category":"articulation","codepoint":"ED44","name":"articSoftAccentTenutoAbove"},{"category":"articulation","codepoint":"ED45","name":"articSoftAccentTenutoBelow"},{"category":"articulation","codepoint":"ED46","name":"articSoftAccentTenutoStaccatoAbove"},{"category":"articulation","codepoint":"ED47","name":"articSoftAccentTenutoStaccatoBelow"},{"category":"articulation","codepoint":"E4A6","name":"articStaccatissimoAbove"},{"category":"articulation","codepoint":"E4A7","name":"articStaccatissimoBelow"},{"category":"articulation","codepoint":"E4AA","name":"articStaccatissimoStrokeAbove"},{"category":"articulation","codepoint":"E4AB","name":"articStaccatissimoStrokeBelow"},{"category":"articulation","codepoint":"E4A8","name":"articStaccatissimoWedgeAbove"},{"category":"articulation","codepoint":"E4A9","name":"articStaccatissimoWedgeBelow"},{"category":"articulation","codepoint":"E4A2","name":"articStaccatoAbove"},{"category":"articulation","codepoint":"E4A3","name":"articStaccatoBelow"},{"category":"articulation","codepoint":"E4B6","name":"articStressAbove"},{"category":"articulation","codepoint":"E4B7","name":"articStressBelow"},{"category":"articulation","codepoint":"E4A4","name":"articTenutoAbove"},{"category":"articulation","codepoint":"E4B4","name":"articTenutoAccentAbove"},{"category":"articulation","codepoint":"E4B5","name":"articTenutoAccentBelow"},{"category":"articulation","codepoint":"E4A5","name":"articTenutoBelow"},{"category":"articulation","codepoint":"E4B2","name":"articTenutoStaccatoAbove"},{"category":"articulation","codepoint":"E4B3","name":"articTenutoStaccatoBelow"},{"category":"articulation","codepoint":"E4B8","name":"articUnstressAbove"},{"category":"articulation","codepoint":"E4B9","name":"articUnstressBelow"},{"category":"breathCaesura","codepoint":"E4CE","name":"breathMarkComma"},{"category":"breathCaesura","codepoint":"E4CF","name":"breathMarkTick"},{"category":"clef","codepoint":"E05C","name":"cClef"},{"category":"clef","codepoint":"E05D","name":"cClef8vb"},{"category":"breathCaesura","codepoint":"E4D1","name":"caesura"},{"category":"breathCaesura","codepoint":"E4D4","name":"caesuraCurved"},{"category":"breathCaesura","codepoint":"E4D3","name":"caesuraShort"},{"category":"breathCaesura","codepoint":"E4D7","name":"caesuraSingleStroke"},{"category":"breathCaesura","codepoint":"E4D2","name":"caesuraThick"},{"category":"clef","codepoint":"E07E","name":"clef15"},{"category":"clef","codepoint":"E07D","name":"clef8"},{"category":"repeatNavigation","codepoint":"E048","name":"coda"},{"category":"repeatNavigation","codepoint":"E049","name":"codaSquare"},{"category":"repeatNavigation","codepoint":"E046","name":"daCapo"},{"category":"repeatNavigation","codepoint":"E045","name":"dalSegno"},{"category":"dynamic","codepoint":"E546","name":"dynamicCombinedSeparatorColon"},{"category":"dynamic","codepoint":"E547","name":"dynamicCombinedSeparatorHyphen"},{"category":"dynamic","codepoint":"E549","name":"dynamicCombinedSeparatorSlash"},{"category":"dynamic","codepoint":"E53E","name":"dynamicCrescendoHairpin"},{"category":"dynamic","codepoint":"E53F","name":"dynamicDiminuendoHairpin"},{"category":"dynamic","codepoint":"E52F","name":"dynamicFF"},{"category":"dynamic","codepoint":"E530","name":"dynamicFFF"},{"category":"dynamic","codepoint":"E531","name":"dynamicFFFF"},{"category":"dynamic","codepoint":"E532","name":"dynamicFFFFF"},{"category":"dynamic","codepoint":"E533","name":"dynamicFFFFFF"},{"category":"dynamic","codepoint":"E522","name":"dynamicForte"},{"category":"dynamic","codepoint":"E534","name":"dynamicFortePiano"},{"category":"dynamic","codepoint":"E535","name":"dynamicForzando"},{"category":"dynamic","codepoint":"E544","name":"dynamicHairpinBracketLeft"},{"category":"dynamic","codepoint":"E545","name":"dynamicHairpinBracketRight"},{"category":"dynamic","codepoint":"E542","name":"dynamicHairpinParenthesisLeft"},{"category":"dynamic","codepoint":"E543","name":"dynamicHairpinParenthesisRight"},{"category":"dynamic","codepoint":"E52D","name":"dynamicMF"},{"category":"dynamic","codepoint":"E52C","name":"dynamicMP"},{"category":"dynamic","codepoint":"E540","name":"dynamicMessaDiVoce"},{"category":"dynamic","codepoint":"E521","name":"dynamicMezzo"},{"category":"dynamic","codepoint":"E526","name":"dynamicNiente"},{"category":"dynamic","codepoint":"E541","name":"dynamicNienteForHairpin"},{"category":"dynamic","codepoint":"E52E","name":"dynamicPF"},{"category":"dynamic","codepoint":"E52B","name":"dynamicPP"},{"category":"dynamic","codepoint":"E52A","name":"dynamicPPP"},{"category":"dynamic","codepoint":"E529","name":"dynamicPPPP"},{"category":"dynamic","codepoint":"E528","name":"dynamicPPPPP"},{"category":"dynamic","codepoint":"E527","name":"dynamicPPPPPP"},{"category":"dynamic","codepoint":"E520","name":"dynamicPiano"},{"category":"dynamic","codepoint":"E523","name":"dynamicRinforzando"},{"category":"dynamic","codepoint":"E53C","name":"dynamicRinforzando1"},{"category":"dynamic","codepoint":"E53D","name":"dynamicRinforzando2"},{"category":"dynamic","codepoint":"E524","name":"dynamicSforzando"},{"category":"dynamic","codepoint":"E536","name":"dynamicSforzando1"},{"category":"dynamic","codepoint":"E538","name":"dynamicSforzandoPianissimo"},{"category":"dynamic","codepoint":"E537","name":"dynamicSforzandoPiano"},{"category":"dynamic","codepoint":"E539","name":"dynamicSforzato"},{"category":"dynamic","codepoint":"E53B","name":"dynamicSforzatoFF"},{"category":"dynamic","codepoint":"E53A","name":"dynamicSforzatoPiano")PMCAT"
    R"PMCAT(},{"category":"dynamic","codepoint":"E525","name":"dynamicZ"},{"category":"clef","codepoint":"E062","name":"fClef"},{"category":"clef","codepoint":"E066","name":"fClef15ma"},{"category":"clef","codepoint":"E063","name":"fClef15mb"},{"category":"clef","codepoint":"E065","name":"fClef8va"},{"category":"clef","codepoint":"E064","name":"fClef8vb"},{"category":"fermata","codepoint":"E4C0","name":"fermataAbove"},{"category":"fermata","codepoint":"E4C1","name":"fermataBelow"},{"category":"fermata","codepoint":"E4C6","name":"fermataLongAbove"},{"category":"fermata","codepoint":"E4C7","name":"fermataLongBelow"},{"category":"fermata","codepoint":"E4CA","name":"fermataLongHenzeAbove"},{"category":"fermata","codepoint":"E4CB","name":"fermataLongHenzeBelow"},{"category":"fermata","codepoint":"E4C4","name":"fermataShortAbove"},{"category":"fermata","codepoint":"E4C5","name":"fermataShortBelow"},{"category":"fermata","codepoint":"E4CC","name":"fermataShortHenzeAbove"},{"category":"fermata","codepoint":"E4CD","name":"fermataShortHenzeBelow"},{"category":"fermata","codepoint":"E4C8","name":"fermataVeryLongAbove"},{"category":"fermata","codepoint":"E4C9","name":"fermataVeryLongBelow"},{"category":"fermata","codepoint":"E4C2","name":"fermataVeryShortAbove"},{"category":"fermata","codepoint":"E4C3","name":"fermataVeryShortBelow"},{"category":"flag","codepoint":"E24F","name":"flag1024thDown"},{"category":"flag","codepoint":"E24E","name":"flag1024thUp"},{"category":"flag","codepoint":"E249","name":"flag128thDown"},{"category":"flag","codepoint":"E248","name":"flag128thUp"},{"category":"flag","codepoint":"E243","name":"flag16thDown"},{"category":"flag","codepoint":"E242","name":"flag16thUp"},{"category":"flag","codepoint":"E24B","name":"flag256thDown"},{"category":"flag","codepoint":"E24A","name":"flag256thUp"},{"category":"flag","codepoint":"E245","name":"flag32ndDown"},{"category":"flag","codepoint":"E244","name":"flag32ndUp"},{"category":"flag","codepoint":"E24D","name":"flag512thDown"},{"category":"flag","codepoint":"E24C","name":"flag512thUp"},{"category":"flag","codepoint":"E247","name":"flag64thDown"},{"category":"flag","codepoint":"E246","name":"flag64thUp"},{"category":"flag","codepoint":"E241","name":"flag8thDown"},{"category":"flag","codepoint":"E240","name":"flag8thUp"},{"category":"clef","codepoint":"E050","name":"gClef"},{"category":"clef","codepoint":"E054","name":"gClef15ma"},{"category":"clef","codepoint":"E051","name":"gClef15mb"},{"category":"clef","codepoint":"E053","name":"gClef8va"},{"category":"clef","codepoint":"E052","name":"gClef8vb"},{"category":"keyboard","codepoint":"E65E","name":"keyboardLeftPedalPictogram"},{"category":"keyboard","codepoint":"E65F","name":"keyboardMiddlePedalPictogram"},{"category":"keyboard","codepoint":"E656","name":"keyboardPedalHalf"},{"category":"keyboard","codepoint":"E65B","name":"keyboardPedalHalf2"},{"category":"keyboard","codepoint":"E65C","name":"keyboardPedalHalf3"},{"category":"keyboard","codepoint":"E673","name":"keyboardPedalHookEnd"},{"category":"keyboard","codepoint":"E672","name":"keyboardPedalHookStart"},{"category":"keyboard","codepoint":"E657","name":"keyboardPedalUpNotch"},{"category":"keyboard","codepoint":"E670","name":"keyboardPlayWithLH"},{"category":"keyboard","codepoint":"E671","name":"keyboardPlayWithLHEnd"},{"category":"keyboard","codepoint":"E66E","name":"keyboardPlayWithRH"},{"category":"keyboard","codepoint":"E66F","name":"keyboardPlayWithRHEnd"},{"category":"keyboard","codepoint":"E660","name":"keyboardRightPedalPictogram"},{"category":"note","codepoint":"E1E6","name":"note1024thDown"},{"category":"note","codepoint":"E1E5","name":"note1024thUp"},{"category":"note","codepoint":"E1E0","name":"note128thDown"},{"category":"note","codepoint":"E1DF","name":"note128thUp"},{"category":"note","codepoint":"E1DA","name":"note16thDown"},{"category":"note","codepoint":"E1D9","name":"note16thUp"},{"category":"note","codepoint":"E1E2","name":"note256thDown"},{"category":"note","codepoint":"E1E1","name":"note256thUp"},{"category":"note","codepoint":"E1DC","name":"note32ndDown"},{"category":"note","codepoint":"E1DB","name":"note32ndUp"},{"category":"note","codepoint":"E1E4","name":"note512thDown"},{"category":"note","codepoint":"E1E3","name":"note512thUp"},{"category":"note","codepoint":"E1DE","name":"note64thDown"},{"category":"note","codepoint":"E1DD","name":"note64thUp"},{"category":"note","codepoint":"E1D8","name":"note8thDown"},{"category":"note","codepoint":"E1D7","name":"note8thUp"},{"category":"note","codepoint":"E1D0","name":"noteDoubleWhole"},{"category":"note","codepoint":"E1D4","name":"noteHalfDown"},{"category":"note","codepoint":"E1D3","name":"noteHalfUp"},{"category":"note","codepoint":"E1D6","name":"noteQuarterDown"},{"category":"note","codepoint":"E1D5","name":"noteQuarterUp"},{"category":"note","codepoint":"E1D2","name":"noteWhole"},{"category":"notehead","codepoint":"E0A4","name":"noteheadBlack"},{"category":"notehead","codepoint":"E0A0","name":"noteheadDoubleWhole"},{"category":"notehead","codepoint":"E0A1","name":"noteheadDoubleWholeSquare"},{"category":"notehead","codepoint":"E0A3","name":"noteheadHalf"},{"category":"notehead","codepoint":"E0A2","name":"noteheadWhole"},{"category":"ornament","codepoint":"E59A","name":"ornamentBottomLeftConcaveStroke"},{"category":"ornament","codepoint":"E59B","name":"ornamentBottomLeftConcaveStrokeLarge"},{"category":"ornament","codepoint":"E59C","name":"ornamentBottomLeftConvexStroke"},{"category":"ornament","codepoint":"E5A7","name":"ornamentBottomRightConcaveStroke"},{"category":"ornament","codepoint":"E5A8","name":"ornamentBottomRightConvexStroke"},{"category":"ornament","codepoint":"E581","name":"ornamentComma"},{"category":"ornament","codepoint":"E57E","name":"ornamentDoubleObliqueLinesAfterNote"},{"category":"ornament","codepoint":"E57D","name":"ornamentDoubleObliqueLinesBeforeNote"},{"category":"ornament","codepoint":"E578","name":"ornamentDownCurve"},{"category":"ornament","codepoint":"E56F","name":"ornamentHaydn"},{"category":"ornament","codepoint":"E592","name":"ornamentHighLeftConcaveStroke"},{"category":"ornament","codepoint":"E593","name":"ornamentHighLeftConvexStroke"},{"category":"ornament","codepoint":"E5A2","name":"ornamentHighRightConcaveStroke"},{"category":"ornament","codepoint":"E5A3","name":"ornamentHighRightConvexStroke"},{"category":"ornament","codepoint":"E576","name":"ornamentHookAfterNote"},{"category":"ornament","codepoint":"E575","name":"ornamentHookBeforeNote"},{"category":"ornament","codepoint":"E572","name":"ornamentLeftFacingHalfCircle"},{"category":"ornament","codepoint":"E574","name":"ornamentLeftFacingHook"},{"category":"ornament","codepoint":"E597","name":"ornamentLeftPlus"},{"category":"ornament","codepoint":"E596","name":"ornamentLeftShakeT"},{"category":"ornament","codepoint":"E594","name":"ornamentLeftVerticalStroke"},{"category":"ornament","codepoint":"E595","name":"ornamentLeftVerticalStrokeWithCross"},{"category":"ornament","codepoint":"E598","name":"ornamentLowLeftConcaveStroke"},{"category":"ornament","codepoint":"E599","name":"ornamentLowLeftConvexStroke"},{"category":"ornament","codepoint":"E5A5","name":"ornamentLowRightConcaveStroke"},{"category":"ornament","codepoint":"E5A6","name":"ornamentLowRightConvexStroke"},{"category":"ornament","codepoint":"E59F","name":"ornamentMiddleVerticalStroke"},{"category":"ornament","codepoint":"E56D","name":"ornamentMordent"},{"category":"ornament","codepoint":"E57C","name":"ornamentObliqueLineAfterNote"},{"category":"ornament","codepoint":"E57B","name":"ornamentObliqueLineBeforeNote"},{"category":"ornament","codepoint":"E580","name":"ornamentObliqueLineHorizAfterNote"},{"category":"ornament","codepoint":"E57F","name":"ornamentObliqueLineHorizBeforeNote"},{"category":"ornament","codepoint":"E588","name":"ornamentPinceCouperin"},{"category":"ornament","codepoint":"E570","name":"ornamentPortDeVoixV"},{"category":"ornament","codepoint":"E5B2","name":"ornamentPrecompAppoggTrill"},{"category":"ornament","codepoint":"E5B3","name":"ornamentPrecompAppoggTrillSuffix"},{"category":"ornament","codepoint":"E5BE","name":"ornamentPrecompCadence"},{"category":"ornament",")PMCAT"
    R"PMCAT(codepoint":"E5C1","name":"ornamentPrecompCadenceUpperPrefix"},{"category":"ornament","codepoint":"E5C2","name":"ornamentPrecompCadenceUpperPrefixTurn"},{"category":"ornament","codepoint":"E5BF","name":"ornamentPrecompCadenceWithTurn"},{"category":"ornament","codepoint":"E5B1","name":"ornamentPrecompDescendingSlide"},{"category":"ornament","codepoint":"E5C0","name":"ornamentPrecompDoubleCadenceLowerPrefix"},{"category":"ornament","codepoint":"E5C3","name":"ornamentPrecompDoubleCadenceUpperPrefix"},{"category":"ornament","codepoint":"E5C4","name":"ornamentPrecompDoubleCadenceUpperPrefixTurn"},{"category":"ornament","codepoint":"E5C7","name":"ornamentPrecompInvertedMordentUpperPrefix"},{"category":"ornament","codepoint":"E5C5","name":"ornamentPrecompMordentRelease"},{"category":"ornament","codepoint":"E5C6","name":"ornamentPrecompMordentUpperPrefix"},{"category":"ornament","codepoint":"E5BC","name":"ornamentPrecompPortDeVoixMordent"},{"category":"ornament","codepoint":"E5B0","name":"ornamentPrecompSlide"},{"category":"ornament","codepoint":"E5B8","name":"ornamentPrecompSlideTrillBach"},{"category":"ornament","codepoint":"E5B5","name":"ornamentPrecompSlideTrillDAnglebert"},{"category":"ornament","codepoint":"E5B6","name":"ornamentPrecompSlideTrillMarpurg"},{"category":"ornament","codepoint":"E5B9","name":"ornamentPrecompSlideTrillMuffat"},{"category":"ornament","codepoint":"E5BA","name":"ornamentPrecompSlideTrillSuffixMuffat"},{"category":"ornament","codepoint":"E5C8","name":"ornamentPrecompTrillLowerSuffix"},{"category":"ornament","codepoint":"E5BB","name":"ornamentPrecompTrillSuffixDandrieu"},{"category":"ornament","codepoint":"E5BD","name":"ornamentPrecompTrillWithMordent"},{"category":"ornament","codepoint":"E5B7","name":"ornamentPrecompTurnTrillBach"},{"category":"ornament","codepoint":"E5B4","name":"ornamentPrecompTurnTrillDAnglebert"},{"category":"ornament","codepoint":"E571","name":"ornamentRightFacingHalfCircle"},{"category":"ornament","codepoint":"E573","name":"ornamentRightFacingHook"},{"category":"ornament","codepoint":"E5A4","name":"ornamentRightVerticalStroke"},{"category":"ornament","codepoint":"E587","name":"ornamentSchleifer"},{"category":"ornament","codepoint":"E582","name":"ornamentShake3"},{"category":"ornament","codepoint":"E584","name":"ornamentShakeMuffat1"},{"category":"ornament","codepoint":"E57A","name":"ornamentShortObliqueLineAfterNote"},{"category":"ornament","codepoint":"E579","name":"ornamentShortObliqueLineBeforeNote"},{"category":"ornament","codepoint":"E56C","name":"ornamentShortTrill"},{"category":"ornament","codepoint":"E590","name":"ornamentTopLeftConcaveStroke"},{"category":"ornament","codepoint":"E591","name":"ornamentTopLeftConvexStroke"},{"category":"ornament","codepoint":"E5A0","name":"ornamentTopRightConcaveStroke"},{"category":"ornament","codepoint":"E5A1","name":"ornamentTopRightConvexStroke"},{"category":"ornament","codepoint":"E56E","name":"ornamentTremblement"},{"category":"ornament","codepoint":"E589","name":"ornamentTremblementCouperin"},{"category":"ornament","codepoint":"E566","name":"ornamentTrill"},{"category":"ornament","codepoint":"E567","name":"ornamentTurn"},{"category":"ornament","codepoint":"E568","name":"ornamentTurnInverted"},{"category":"ornament","codepoint":"E569","name":"ornamentTurnSlash"},{"category":"ornament","codepoint":"E56A","name":"ornamentTurnUp"},{"category":"ornament","codepoint":"E56B","name":"ornamentTurnUpS"},{"category":"ornament","codepoint":"E577","name":"ornamentUpCurve"},{"category":"ornament","codepoint":"E583","name":"ornamentVerticalLine"},{"category":"ornament","codepoint":"E59D","name":"ornamentZigZagLineNoRightEnd"},{"category":"ornament","codepoint":"E59E","name":"ornamentZigZagLineWithRightEnd"},{"category":"octave","codepoint":"E510","name":"ottava"},{"category":"octave","codepoint":"E511","name":"ottavaAlta"},{"category":"octave","codepoint":"E512","name":"ottavaBassa"},{"category":"octave","codepoint":"E513","name":"ottavaBassaBa"},{"category":"octave","codepoint":"E51C","name":"ottavaBassaVb"},{"category":"octave","codepoint":"E514","name":"quindicesima"},{"category":"octave","codepoint":"E515","name":"quindicesimaAlta"},{"category":"octave","codepoint":"E516","name":"quindicesimaBassa"},{"category":"octave","codepoint":"E51D","name":"quindicesimaBassaMb"},{"category":"repeatNavigation","codepoint":"E500","name":"repeat1Bar"},{"category":"repeatNavigation","codepoint":"E501","name":"repeat2Bars"},{"category":"repeatNavigation","codepoint":"E502","name":"repeat4Bars"},{"category":"repeatNavigation","codepoint":"E505","name":"repeatBarLowerDot"},{"category":"repeatNavigation","codepoint":"E504","name":"repeatBarSlash"},{"category":"repeatNavigation","codepoint":"E503","name":"repeatBarUpperDot"},{"category":"repeatNavigation","codepoint":"E044","name":"repeatDot"},{"category":"repeatNavigation","codepoint":"E043","name":"repeatDots"},{"category":"repeatNavigation","codepoint":"E040","name":"repeatLeft"},{"category":"repeatNavigation","codepoint":"E041","name":"repeatRight"},{"category":"repeatNavigation","codepoint":"E042","name":"repeatRightLeft"},{"category":"rest","codepoint":"E4ED","name":"rest1024th"},{"category":"rest","codepoint":"E4EA","name":"rest128th"},{"category":"rest","codepoint":"E4E7","name":"rest16th"},{"category":"rest","codepoint":"E4EB","name":"rest256th"},{"category":"rest","codepoint":"E4E8","name":"rest32nd"},{"category":"rest","codepoint":"E4EC","name":"rest512th"},{"category":"rest","codepoint":"E4E9","name":"rest64th"},{"category":"rest","codepoint":"E4E6","name":"rest8th"},{"category":"rest","codepoint":"E4E2","name":"restDoubleWhole"},{"category":"rest","codepoint":"E4E4","name":"restHalf"},{"category":"rest","codepoint":"E4E1","name":"restLonga"},{"category":"rest","codepoint":"E4E5","name":"restQuarter"},{"category":"rest","codepoint":"E4E3","name":"restWhole"},{"category":"repeatNavigation","codepoint":"E047","name":"segno"},{"category":"repeatNavigation","codepoint":"E04A","name":"segnoSerpent1"},{"category":"repeatNavigation","codepoint":"E04B","name":"segnoSerpent2"},{"category":"octave","codepoint":"E517","name":"ventiduesima"},{"category":"octave","codepoint":"E518","name":"ventiduesimaAlta"},{"category":"octave","codepoint":"E519","name":"ventiduesimaBassa"},{"category":"octave","codepoint":"E51E","name":"ventiduesimaBassaMb"},{"category":"arpeggio","codepoint":"EAAA","name":"wiggleArpeggiatoDown"},{"category":"arpeggio","codepoint":"EAAE","name":"wiggleArpeggiatoDownArrow"},{"category":"arpeggio","codepoint":"EAAC","name":"wiggleArpeggiatoDownSwash"},{"category":"arpeggio","codepoint":"EAA9","name":"wiggleArpeggiatoUp"},{"category":"arpeggio","codepoint":"EAAD","name":"wiggleArpeggiatoUpArrow"},{"category":"arpeggio","codepoint":"EAAB","name":"wiggleArpeggiatoUpSwash"},{"category":"glissando","codepoint":"EAAF","name":"wiggleGlissando"},{"category":"ornament","codepoint":"EAA4","name":"wiggleTrill"}]},"acceptedStructuralContainers":["mei","music","body","mdiv","score","scoreDef","section","ending","measure","staff"],"arpeggioGeometry":{"brackets":"order=nonarp, arrow=false, solid","coordinates":"page inches, bottom-left origin, Y-up, CCW degrees","members":"hash references to emitted existing written note IDs; no synthesized identity","normal":"order=up, arrow=false, wavy","stageEvidence":"Canonical version 1 geometry is parsed and rendered; playback, input and scoring remain unverified.","straight":"explicit order, arrow=true, solid, pm:arpeggio-style=straight","up_down":"explicit order, arrow=true, wavy","version":1},"articulationStageEvidence":{"input":"unverified","model":"existing_component_models_source_inspected","parse":"Original five-token component parser inspected against original source; token groups are already supported.","playback":"unverified","render":"existing_component_and_composite_paths_source_inspected; no new feature claim","scoring":"unverified"},"capabilityCatalogVersion":"pianomania.notation-capabilities.v1","creationPolicy":"Independent of compatibility admission. Preserve imported items. Pending features stay available.","evidenceBoundary":"Catalog records inspected source behavior.)PMCAT"
    R"PMCAT( It does not attest rendered, MIDI, input, scoring or device acceptance.","fatalRules":["malformed_xml","duplicate_identity","invalid_note_identity_pitch_timing","unsupported_playable_wrapper","invalid_tie_anchor","unbound_ordinary_midi_occurrence","ambiguous_traversal"],"fidelityReviews":[{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"source-visibility","humanName":"Source visibility","observedBehavior":"unverified","reasonCode":"source_visibility_requires_correspondence","stage":"export"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"rehearsal-mark","humanName":"Rehearsal mark fidelity","observedBehavior":"unverified","reasonCode":"rehearsal_fidelity_unverified","stage":"render"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"featureId":"symbol-parentheses","humanName":"Parenthesis and enclosure fidelity","observedBehavior":"unverified","reasonCode":"enclosure_fidelity_unverified","stage":"export"},{"affectedModes":["practice","game_solo","flow"],"featureId":"note-playback-disabled","humanName":"Playback-disabled note performance","observedBehavior":"unverified","reasonCode":"muted_note_performance_unverified","stage":"playback"}],"glissandoGeometry":{"elements":["gliss"],"glyphNames":["wiggleGlissando"],"hiddenPolicy":"Valid hidden geometry is supported and has no visual omission.","lineForms":["solid","wavy"],"performanceEvidence":"Playback, input and scoring remain unverified.","segmentPolicy":"Exactly one placed page segment with exact written-note anchors is currently rendered."},"hostParity":{"generator":"scripts/generate-notation-capabilities.py","requiredCheck":"--check --host all after Unity source release; --check --host helper while Unity frozen","rule":"Each feature support change updates this catalog and regenerates both hosts in one integration; no host may silently promote a stage from unverified. Pending Composer implementation must consume this same version and vocabulary."},"modes":["practice","game_solo","flow","native_preview","webgl_preview"],"parsedArticulationCombinations":["acc stacc","marc stacc","marc ten","acc ten","stacc ten"],"parsedArticulationTokens":["stacc","stacciss","ten","marc","acc","spicc","stroke","acc-soft","stress","unstress","lv"],"pianoSymbolGeometry":{"elements":["fermata","ornam","trill","mordent","turn","breath","caesura"],"glyphFamilies":{"breath":["breathMark"],"caesura":["caesura"],"fermata":["fermata"],"ornament":["ornament"],"symbol":[]},"hiddenPolicy":"Valid hidden geometry is supported and has no visual omission.","performanceEvidence":"Playback, input and scoring remain unverified.","requiredAttributes":["pm:piano-symbol-kind","pm:piano-symbol-version","pm:piano-symbol-page","pm:piano-symbol-visible","pm:piano-symbol-center","pm:piano-symbol-bounds","pm:piano-symbol-ink-bounds"],"version":1},"recognizedOrnaments":["ornam","trill","mordent","turn"],"runtimeOmissions":[{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"harm","featureId":"harmony","humanName":"Chord symbols","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"fb","featureId":"figured-bass","humanName":"Figured bass","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"},{"affectedModes":["practice","game_solo","flow","native_preview","webgl_preview"],"element":"verse","featureId":"lyrics","humanName":"Lyrics","omittedBehavior":["visual"],"performanceCompleteness":"unverified","stage":"parse"}],"schemaVersion":"pianomania.notation-capability-catalog.v1","sourceVariantComparisons":{"arpeggio":{"meiElement":"arpeg","meiProperties":["order","form","arrow","startid","plist","pm:arpeggio-style","pm:arpeggio-play","lform","pm:arpeggio-members","pm:arpeggio-geometry-version","pm:arpeggio-page","pm:arpeggio-visible","pm:arpeggio-bounds","pm:arpeggio-line-width","pm:arpeggio-lines","pm:arpeggio-glyphs"],"sourceElement":"Arpeggio","sourceProperty":"subtype","variants":{"0":"normal","1":"up","2":"down","3":"bracket","4":"up-straight","5":"down-straight"}}},"stages":["source","export","parse","model","render","playback","input","scoring","traversal"],"supportedArticulationCombinations":["acc stacc","marc stacc","marc ten","acc ten","stacc ten"],"supportedArticulationTokens":["stacc","stacciss","ten","marc","acc","spicc","stroke","acc-soft","stress","unstress","lv"],"topology":{"currentRuntimeStaffIds":["1","2"],"featureId":"piano-staff-topology","importedEditing":"preserved","migration":"Preserve N source/display staff identities and voices. Note hand remains independent. Never silently collapse a playable staff.","sourceCreation":"allowed","unsupportedExportedIdentity":"fatal"},"unsupportedArticulationNames":{}})PMCAT";
// END GENERATED NOTATION CATALOG

QJsonObject catalog()
{
    return QJsonDocument::fromJson(NOTATION_CATALOG_JSON).object();
}
QString sha256(const QByteArray& bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}
QByteArray asciiJson(QByteArray value)
{
    QString unicode = QString::fromUtf8(value);
    QByteArray result;
    for (QChar character : unicode) {
        if (character.unicode() < 128) result += char(character.unicode());
        else result += QByteArray("\\u") + QByteArray::number(character.unicode(), 16).rightJustified(4, '0');
    }
    return result;
}
QString itemIdentity(const mu::engraving::EngravingItem* item)
{
    return item && item->eid().isValid() ? QString::fromStdString(item->eid().toStdString()) : QString();
}
QJsonArray strings(const QStringList& values)
{
    QJsonArray result;
    for (const QString& value : values) result.append(value);
    return result;
}
QStringList values(const QJsonArray& array)
{
    QStringList result;
    for (const QJsonValue& value : array) result.append(value.toString());
    return result;
}
QJsonObject reportBase(const QString& sourcePath)
{
    auto capabilities = catalog();
    QStringList modes = values(capabilities["modes"].toArray());
    std::sort(modes.begin(), modes.end());
    QJsonObject completeness;
    for (const QString& mode : modes) {
        QJsonObject stages;
        for (const QJsonValue& value : capabilities["stages"].toArray()) {
            QString stage = value.toString();
            stages[stage] = mode.endsWith("_preview") && (stage == "input" || stage == "scoring")
                ? "not_applicable" : "unverified";
        }
        stages["source"] = "partial";
        completeness[mode] = stages;
    }
    QFile source(sourcePath);
    QString sourceDigest;
    if (source.exists() && source.open(QIODevice::ReadOnly)) sourceDigest = sha256(source.readAll());
    return {
        {"schemaVersion", "pianomania.notation-compatibility-report.v1"},
        {"capabilityCatalogVersion", capabilities["capabilityCatalogVersion"]},
        {"capabilityCatalogDigest", NOTATION_CATALOG_DIGEST},
        {"scoreIdentity", QFileInfo(sourcePath).fileName()}, {"sourceDigest", sourceDigest}, {"meiDigest", ""},
        {"exporterProvenance", "native-composer-compatibility-v1"},
        {"inspectedStages", QJsonArray{"source"}}, {"targetModes", strings(modes)},
        {"completeness", completeness}, {"findings", QJsonArray()}
    };
}
void finishReport(QJsonObject& report)
{
    QJsonArray findings = report["findings"].toArray();
    QList<QJsonObject> sorted;
    for (const QJsonValue& value : findings) sorted.append(value.toObject());
    std::sort(sorted.begin(), sorted.end(), [](const auto& left, const auto& right) {
        return left["findingId"].toString() < right["findingId"].toString();
    });
    findings = {};
    for (const auto& item : sorted) findings.append(item);
    report["findings"] = findings;
    report.remove("reportDigest");
    report["reportDigest"] = sha256(mu::project::notationcompatibility::canonicalJson(report));
}
void addFinding(QJsonObject& report, const QString& feature, const QString& variant,
                const QString& human, const QString& reason, const QJsonObject& location,
                const QString& stage, const QString& unit, bool fatal = false,
                const QString& behavior = "", const QJsonArray& omitted = QJsonArray{"visual"},
                const QJsonArray& affectedModes = {})
{
    QString observed = behavior.isEmpty() ? (fatal ? "rejected" : "omitted") : behavior;
    QJsonArray modes = report["targetModes"].toArray();
    if (!affectedModes.isEmpty()) {
        QJsonArray selected;
        for (const QJsonValue& mode : modes) if (affectedModes.contains(mode)) selected.append(mode);
        modes = selected;
    }
    if (modes.isEmpty()) return;
    QJsonArray key {feature, variant, fatal ? "fatal" : "warning", reason, stage, modes, observed, omitted, unit};
    QString id = sha256(asciiJson(QJsonDocument(key).toJson(QJsonDocument::Compact)));
    QJsonArray findings = report["findings"].toArray();
    int found = -1;
    for (int index = 0; index < findings.size(); ++index) {
        if (findings[index].toObject()["findingId"] == id) { found = index; break; }
    }
    QJsonObject row = found >= 0 ? findings[found].toObject() : QJsonObject {
        {"findingId", id}, {"featureId", feature}, {"variant", variant}, {"humanName", human},
        {"severity", fatal ? "fatal" : "warning"}, {"reasonCode", reason}, {"stage", stage},
        {"affectedModes", modes}, {"observedBehavior", observed}, {"omittedBehavior", omitted},
        {"countUnit", unit}, {"count", 0}, {"locations", QJsonArray()}, {"representativeExamples", QJsonArray()}
    };
    QJsonArray locations = row["locations"].toArray(); locations.append(location);
    QJsonArray examples = row["representativeExamples"].toArray();
    if (examples.size() < 3) examples.append(location["itemIdentity"]);
    row["count"] = locations.size(); row["locations"] = locations; row["representativeExamples"] = examples;
    if (found >= 0) findings[found] = row; else findings.append(row);
    report["findings"] = findings;
}
struct XmlItem {
    QString name;
    QString namespaceUri;
    QString text;
    QJsonObject attributes;
    QJsonObject location;
    int parent = -1;
};
// Match the placed single-segment display contract; stage acceptance stays unverified.
QString glissandoDisplayOmission(const QList<XmlItem>& items, int index,
                                 const QHash<QString, int>& byId, bool& textOmitted)
{
    const auto& attributes = items[index].attributes;
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    QList<int> segments;
    for (int child = index + 1; child < items.size(); ++child)
        if (items[child].parent == index && items[child].name == "gliss-segment"
            && items[child].namespaceUri == "http://pianomania.gg/ns/mei") segments.append(child);
    if (segments.isEmpty()) return "glissando_geometry_missing";
    if (segments.size() != 1) return "glissando_multisegment_omission";
    const auto number = [](const QString& value, double& result) {
        bool valid = false; result = value.toDouble(&valid);
        return !value.isEmpty() && valid && std::isfinite(result);
    };
    const auto tuple = [&number](const QString& value, int count, QList<double>& result) {
        const auto fields = value.split(',');
        if (fields.size() != count) return false;
        result.clear();
        for (const auto& field : fields) {
            double parsed = 0; if (!number(field, parsed)) return false;
            result.append(parsed);
        }
        return true;
    };
    QList<double> start, end, center;
    const QString form = attributes["lform"].toString();
    const auto flag = [](const QString& value) { return value == "true" || value == "false"; };
    const auto anchor = [&byId, &items](QString value) {
        if (!value.startsWith('#') || value.size() < 2
            || std::any_of(value.cbegin(), value.cend(), [](QChar c) { return c.isSpace(); })) return false;
        value.remove(0, 1);
        return byId.contains(value) && items[byId.value(value)].name == "note";
    };
    if (!anchor(attributes["startid"].toString()) || !anchor(attributes["endid"].toString()))
        return "glissando_anchor_unrecognized";
    if ((form != "solid" && form != "wavy")
        || !flag(attributes["visible"].toString()) || !flag(attributes[prefix + "gliss-play"].toString())
        || !tuple(attributes[prefix + "gliss-start-center"].toString(), 2, center)
        || !tuple(attributes[prefix + "gliss-end-center"].toString(), 2, center)) return "glissando_geometry_unrecognized";
    const int segmentIndex = segments.first();
    const auto segment = items[segmentIndex].attributes;
    bool validPage = false; int page = segment["page"].toString().toInt(&validPage);
    double width = 0;
    if (!validPage || page < 0 || !tuple(segment["start"].toString(), 2, start)
        || !tuple(segment["end"].toString(), 2, end) || start == end
        || !number(segment["width"].toString(), width) || width <= 0) return "glissando_geometry_unrecognized";
    for (int child = segmentIndex + 1; child < items.size(); ++child) {
        if (items[child].parent != segmentIndex) continue;
        const auto& ink = items[child];
        const auto inkAttributes = ink.attributes;
        if (ink.name != "glyph" && ink.name != "text") return "glissando_geometry_unrecognized";
        const QString font = inkAttributes["font"].toString();
        const auto corners = inkAttributes["corners"].toString().split(';');
        QList<QList<double>> points;
        if (font.isEmpty() || corners.size() != 4) return "glissando_geometry_unrecognized";
        for (const auto& corner : corners) {
            QList<double> point;
            if (!tuple(corner, 2, point)) return "glissando_geometry_unrecognized";
            points.append(point);
        }
        const auto& a = points[0]; const auto& b = points[1]; const auto& c = points[2];
        if ((b[0] - a[0]) * (c[1] - a[1]) == (b[1] - a[1]) * (c[0] - a[0])) return "glissando_geometry_unrecognized";
        if (ink.name == "glyph") {
            if (inkAttributes["name"] != "wiggleGlissando"
                || (form == "wavy" && font != "Leland" && font != "Bravura")) return "glissando_geometry_unrecognized";
        } else {
            bool validStyle = false; int style = inkAttributes["font-style"].toString().toInt(&validStyle);
            if (ink.text.isEmpty() || !validStyle || style < 0 || style > 15) return "glissando_geometry_unrecognized";
            const bool supported = ((font == "Edwin" && style == 2) || (font == "Academico" && style == 1))
                && std::all_of(ink.text.cbegin(), ink.text.cend(), [](QChar c) { return c >= ' ' && c <= '~'; });
            textOmitted |= !supported;
        }
    }
    return {};
}
QString arpeggioVariant(const QJsonObject& attributes)
{
    QString order = attributes["order"].toString("normal");
    if (order == "nonarp") return "bracket";
    QString style = attributes["{http://pianomania.gg/ns/mei}arpeggio-style"].toString();
    bool straight = style == "straight" || attributes["form"] == "straight" || attributes["lform"] == "solid";
    if (order == "up" && attributes["arrow"] == "false" && (style == "wavy" || attributes["lform"] == "wavy")) return "normal";
    return straight ? order + "-straight" : order;
}

bool finiteTuple(const QString& value, int count, QList<double>& result)
{
    const QStringList fields = value.split(',');
    if (fields.size() != count) return false;
    result.clear();
    for (const QString& field : fields) {
        bool valid = false; const double parsed = field.toDouble(&valid);
        if (!valid || !std::isfinite(parsed)) return false;
        result.append(parsed);
    }
    return true;
}

QString typedSymbolGeometryIssue(const XmlItem& item, const QString& kind)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    if (!item.attributes.contains(prefix + "piano-symbol-kind")
        || !item.attributes.contains(prefix + "piano-symbol-version"))
        return kind + "_geometry_missing";
    if (item.attributes[prefix + "piano-symbol-kind"] != kind
        || item.attributes[prefix + "piano-symbol-version"] != "1")
        return kind + "_geometry_unrecognized";
    bool pageValid = false;
    const int page = item.attributes[prefix + "piano-symbol-page"].toString().toInt(&pageValid);
    const QString visible = item.attributes[prefix + "piano-symbol-visible"].toString();
    QList<double> center, bounds, ink;
    if (!pageValid || page <= 0 || !QStringList{"true", "false"}.contains(visible)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-center"].toString(), 2, center)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-bounds"].toString(), 4, bounds)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-ink-bounds"].toString(), 4, ink)
        || bounds[0] > bounds[2] || bounds[3] > bounds[1] || ink[0] > ink[2] || ink[3] > ink[1]
        || ink[0] < bounds[0] || ink[2] > bounds[2] || ink[1] > bounds[1] || ink[3] < bounds[3]
        || std::abs((ink[0] + ink[2]) / 2 - center[0]) > 0.0001
        || std::abs((ink[1] + ink[3]) / 2 - center[1]) > 0.0001
        || (visible == "true" && (ink[0] == ink[2] || ink[1] == ink[3])))
        return kind + "_geometry_unrecognized";
    const bool hasAnchorCenter = item.attributes.contains(prefix + "piano-symbol-anchor-center");
    const bool hasSpatium = item.attributes.contains(prefix + "piano-symbol-spatium");
    if (hasAnchorCenter != hasSpatium) return kind + "_geometry_unrecognized";
    if (hasAnchorCenter) {
        QList<double> anchorCenter;
        bool spatiumValid = false;
        const double spatium = item.attributes[prefix + "piano-symbol-spatium"].toString().toDouble(&spatiumValid);
        if (!finiteTuple(item.attributes[prefix + "piano-symbol-anchor-center"].toString(), 2, anchorCenter)
            || !spatiumValid || !std::isfinite(spatium) || spatium <= 0)
            return kind + "_geometry_unrecognized";
    }
    return {};
}

QString dynamicVisualIssue(const XmlItem& item, const QJsonObject& capabilities)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    if (!item.attributes.contains(prefix + "piano-symbol-glyphs")) return "dynamic_glyph_run_missing";
    const QStringList run = item.attributes[prefix + "piano-symbol-glyphs"].toString().split(';', Qt::KeepEmptyParts);
    QSet<QString> accepted;
    for (const QJsonValue& value : capabilities["acceptedPianoGlyphCatalog"].toObject()["symbols"].toArray()) {
        const QJsonObject glyph = value.toObject();
        if (glyph["category"] == "dynamic") accepted.insert(glyph["name"].toString());
    }
    if (run.isEmpty() || std::any_of(run.cbegin(), run.cend(), [&](const QString& name) {
        return name.isEmpty() || !accepted.contains(name);
    })) return "dynamic_glyph_run_unrecognized";
    const QString explicitName = item.attributes["glyph.name"].toString();
    if (!explicitName.isEmpty() && (run.size() != 1 || run.first() != explicitName
        || item.attributes["glyph.auth"].toString().compare("smufl", Qt::CaseInsensitive) != 0))
        return "dynamic_glyph_run_unrecognized";
    return typedSymbolGeometryIssue(item, "dynamic");
}

QString articulationVisualIssue(const XmlItem& item, const QJsonObject& capabilities)
{
    const QString name = item.attributes["glyph.name"].toString();
    bool accepted = false;
    for (const QJsonValue& value : capabilities["acceptedPianoGlyphCatalog"].toObject()["symbols"].toArray()) {
        const QJsonObject glyph = value.toObject();
        if (glyph["name"] == name && (glyph["category"] == "articulation" || glyph["category"] == "fermata")) {
            accepted = true; break;
        }
    }
    if (!accepted || item.attributes["glyph.auth"].toString().compare("smufl", Qt::CaseInsensitive) != 0)
        return "articulation_glyph_unrecognized";
    return typedSymbolGeometryIssue(item, "articulation");
}

QString arpeggioVisualIssue(const QJsonObject& attributes, const QHash<QString, int>& byId,
                            const QJsonObject& capabilities)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    const auto reference = [&byId](QString value) {
        if (!value.startsWith('#') || value.size() < 2 || value.contains(QRegularExpression("\\s"))) return false;
        value.remove(0, 1); return byId.contains(value);
    };
    if (!reference(attributes["startid"].toString())) return "arpeggio_anchor_unrecognized";
    for (const QString& value : attributes["plist"].toString().split(' ', Qt::SkipEmptyParts))
        if (!reference(value)) return "arpeggio_anchor_unrecognized";
    const QString order = attributes["order"].toString();
    const QString style = attributes[prefix + "arpeggio-style"].toString();
    if (!QStringList{"up", "down", "nonarp"}.contains(order)
        || !QStringList{"wavy", "straight", "bracket"}.contains(style)
        || (style == "bracket") != (order == "nonarp")
        || !QStringList{"true", "false"}.contains(attributes["arrow"].toString())
        || !QStringList{"true", "false"}.contains(attributes[prefix + "arpeggio-play"].toString()))
        return "arpeggio_semantics_unrecognized";
    if (!attributes.contains(prefix + "arpeggio-geometry-version")) return "arpeggio_geometry_missing";
    if (attributes[prefix + "arpeggio-geometry-version"] != "1") return "arpeggio_geometry_unrecognized";
    bool pageValid = false, widthValid = false;
    const int page = attributes[prefix + "arpeggio-page"].toString().toInt(&pageValid);
    const double width = attributes[prefix + "arpeggio-line-width"].toString().toDouble(&widthValid);
    const QString visible = attributes[prefix + "arpeggio-visible"].toString();
    QList<double> tuple;
    if (!pageValid || page <= 0 || !widthValid || !std::isfinite(width) || width <= 0
        || !QStringList{"true", "false"}.contains(visible)
        || !finiteTuple(attributes[prefix + "xy"].toString(), 2, tuple)
        || !finiteTuple(attributes[prefix + "arpeggio-bounds"].toString(), 4, tuple)
        || tuple[0] > tuple[2] || tuple[1] < tuple[3]
        || !QStringList{"Leland", "Bravura"}.contains(attributes[prefix + "arpeggio-font"].toString())
        || attributes[prefix + "arpeggio-line-cap"] != "flat"
        || !attributes.contains(prefix + "arpeggio-lines") || !attributes.contains(prefix + "arpeggio-glyphs"))
        return "arpeggio_geometry_unrecognized";
    int primitives = 0;
    for (const QString& line : attributes[prefix + "arpeggio-lines"].toString().split(';', Qt::SkipEmptyParts)) {
        if (!finiteTuple(line, 4, tuple) || (tuple[0] == tuple[2] && tuple[1] == tuple[3]))
            return "arpeggio_geometry_unrecognized";
        ++primitives;
    }
    QSet<QString> names;
    for (const QJsonValue& value : capabilities["acceptedPianoGlyphCatalog"].toObject()["symbols"].toArray()) {
        const auto glyph = value.toObject(); if (glyph["category"] == "arpeggio") names.insert(glyph["name"].toString());
    }
    for (const QString& glyph : attributes[prefix + "arpeggio-glyphs"].toString().split(';', Qt::SkipEmptyParts)) {
        QStringList fields = glyph.split(',');
        if (fields.size() != 8 || !names.contains(fields.first())
            || !finiteTuple(fields.mid(1).join(','), 7, tuple)) return "arpeggio_geometry_unrecognized";
        ++primitives;
    }
    const bool isVisible = visible == "true";
    if ((isVisible && primitives == 0) || (!isVisible && primitives != 0)) return "arpeggio_geometry_unrecognized";
    if (isVisible) {
        const QStringList members = attributes[prefix + "arpeggio-members"].toString().split(' ', Qt::SkipEmptyParts);
        const QStringList centers = attributes[prefix + "arpeggio-member-centers"].toString().split(';', Qt::SkipEmptyParts);
        QSet<QString> unique;
        if (!finiteTuple(attributes[prefix + "arpeggio-anchor-center"].toString(), 2, tuple)
            || members.isEmpty() || members.size() != centers.size()) return "arpeggio_registration_geometry_missing";
        for (const QString& member : members)
            if (!reference(member) || unique.contains(member)) return "arpeggio_registration_geometry_missing";
            else unique.insert(member);
        for (const QString& center : centers)
            if (!finiteTuple(center, 2, tuple)) return "arpeggio_registration_geometry_missing";
    }
    return {};
}

QString pianoSymbolVisualIssue(const XmlItem& item, const QJsonObject& capabilities)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    const QMap<QString, QString> kinds{{"fermata", "fermata"}, {"ornam", "ornament"}, {"trill", "ornament"},
        {"mordent", "ornament"}, {"turn", "ornament"}, {"breath", "breath"}, {"caesura", "caesura"}};
    QString kind = kinds.value(item.name);
    const QString declaredKind = item.attributes[prefix + "piano-symbol-kind"].toString();
    if (item.name == "ornam" && declaredKind == "symbol") kind = "symbol";
    if (kind.isEmpty() || declaredKind != kind)
        return "piano_symbol_kind_unrecognized";
    QString name = item.attributes["glyph.name"].toString();
    const bool explicitGlyph = !name.isEmpty() || !item.attributes["glyph.num"].toString().isEmpty();
    QHash<QString, QString> byCodepoint;
    QSet<QString> accepted;
    for (const QJsonValue& value : capabilities["acceptedPianoGlyphCatalog"].toObject()["symbols"].toArray()) {
        const auto glyph = value.toObject(); accepted.insert(glyph["name"].toString());
        byCodepoint.insert(glyph["codepoint"].toString(), glyph["name"].toString());
    }
    if (!item.attributes["glyph.num"].toString().isEmpty()) {
        QString number = item.attributes["glyph.num"].toString(); bool valid = false; int base = 10;
        if (number.startsWith("U+", Qt::CaseInsensitive) || number.startsWith("0x", Qt::CaseInsensitive)) { number.remove(0, 2); base = 16; }
        else if (number.contains(QRegularExpression("[A-Fa-f]"))) base = 16;
        const uint codepoint = number.toUInt(&valid, base);
        const QString numbered = valid ? byCodepoint.value(QString::number(codepoint, 16).toUpper().rightJustified(4, '0')) : QString();
        if (numbered.isEmpty() || (!name.isEmpty() && name != numbered)) return "piano_symbol_glyph_unrecognized";
        name = numbered;
    }
    if (name.isEmpty()) {
        const QString form = item.attributes["form"].toString().toLower();
        const QString shape = item.attributes["shape"].toString().toLower();
        const bool isLong = item.attributes["long"] == "true";
        bool below = item.attributes["place"] == "below";
        if (!below && item.attributes.contains("yOffset")) {
            bool valid = false; const double offset = item.attributes["yOffset"].toString().toDouble(&valid);
            below = valid && std::isfinite(offset) && offset < 0;
        }
        const QString vertical = below ? "Below" : "Above";
        if (item.name == "fermata") {
            if (shape.isEmpty() || shape == "curved") name = "fermata" + vertical;
            else if (shape == "angular") name = "fermataShort" + vertical;
            else if (shape == "square") name = "fermataLong" + vertical;
        } else if (item.name == "breath") {
            const QMap<QString, QString> names{{"", "breathMarkComma"}, {"comma", "breathMarkComma"},
                {"tick", "breathMarkTick"}, {"upbow", "breathMarkUpbow"}, {"salzedo", "breathMarkSalzedo"}};
            name = names.value(form);
        } else if (item.name == "caesura") {
            const QMap<QString, QString> names{{"", "caesura"}, {"normal", "caesura"}, {"thick", "caesuraThick"},
                {"short", "caesuraShort"}, {"curved", "caesuraCurved"}};
            name = names.value(form);
        } else if (item.name == "trill") name = isLong ? "ornamentTremblement" : "ornamentTrill";
        else if (item.name == "mordent") {
            if (form == "upper") name = isLong ? "ornamentTremblement" : "ornamentShortTrill";
            else if (form.isEmpty() || form == "lower") name = isLong ? "ornamentPrecompTrillWithMordent" : "ornamentMordent";
        } else if (item.name == "turn") {
            if (form.isEmpty() || form == "upper") name = "ornamentTurn";
            else if (QStringList{"lower", "inv", "inverted"}.contains(form)) name = "ornamentTurnInverted";
        }
    }
    const QString family = kind == "breath" ? "breathMark" : kind == "symbol" ? QString() : kind;
    if ((explicitGlyph && item.attributes["glyph.auth"].toString().compare("smufl", Qt::CaseInsensitive) != 0)
        || !accepted.contains(name) || (!family.isEmpty() && !name.startsWith(family))) return "piano_symbol_glyph_unrecognized";
    if (kind == "symbol") {
        const QString owner = item.attributes[prefix + "piano-symbol-owner"].toString();
        const QString role = item.attributes[prefix + "piano-symbol-role"].toString();
        if ((!owner.isEmpty() || !role.isEmpty())
            && (!QRegularExpression("^[A-Za-z_][A-Za-z0-9._-]*$").match(owner).hasMatch()
            || !QStringList{"begin", "end", "continue"}.contains(role))
        )
            return "piano_symbol_owner_unrecognized";
    }
    if (!item.attributes.contains(prefix + "piano-symbol-version")) return "piano_symbol_geometry_missing";
    if (item.attributes[prefix + "piano-symbol-version"] != "1") return "piano_symbol_geometry_unrecognized";
    bool pageValid = false; const int page = item.attributes[prefix + "piano-symbol-page"].toString().toInt(&pageValid);
    const QString visible = item.attributes[prefix + "piano-symbol-visible"].toString();
    QList<double> center, bounds, ink;
    if (!pageValid || page <= 0 || !QStringList{"true", "false"}.contains(visible)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-center"].toString(), 2, center)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-bounds"].toString(), 4, bounds)
        || !finiteTuple(item.attributes[prefix + "piano-symbol-ink-bounds"].toString(), 4, ink)
        || bounds[0] > bounds[2] || bounds[3] > bounds[1] || ink[0] > ink[2] || ink[3] > ink[1]
        || ink[0] < bounds[0] || ink[2] > bounds[2] || ink[1] > bounds[1] || ink[3] < bounds[3]
        || std::abs((ink[0] + ink[2]) / 2 - center[0]) > 0.0001
        || std::abs((ink[1] + ink[3]) / 2 - center[1]) > 0.0001
        || (visible == "true" && (ink[0] == ink[2] || ink[1] == ink[3])))
        return "piano_symbol_geometry_unrecognized";
    return {};
}
}

QByteArray mu::project::notationcompatibility::canonicalJson(const QJsonObject& object)
{
    return asciiJson(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

SourceInventory mu::project::notationcompatibility::inventoryScore(mu::engraving::Score* score, const QString& sourcePath)
{
    SourceInventory inventory;
    inventory.report = reportBase(sourcePath);
    if (!score) {
        auto completeness = inventory.report["completeness"].toObject();
        for (const QString& mode : completeness.keys()) {
            auto stages = completeness[mode].toObject(); stages["source"] = "unverified"; completeness[mode] = stages;
        }
        inventory.report["completeness"] = completeness;
        inventory.report["inspectedStages"] = QJsonArray();
        finishReport(inventory.report); return inventory;
    }
    using namespace mu::engraving;
    QSet<const EngravingItem*> visited;
    QList<Spanner*> authoredGlissandi;
    QSet<const Spanner*> collectedGlissandi;
    auto collectGlissando = [&](Spanner* spanner) {
        if (!spanner || !spanner->isGlissando() || collectedGlissandi.contains(spanner)) return;
        collectedGlissandi.insert(spanner);
        authoredGlissandi.append(spanner);
    };
    QJsonArray content;
    const QJsonObject capabilities = catalog();
    const QStringList variants {"normal", "up", "down", "bracket", "up-straight", "down-straight"};
    auto visit = [&](EngravingItem* item) {
        if (item && item->isSpannerSegment()) {
            Spanner* spanner = toSpannerSegment(item)->spanner();
            if (spanner && spanner->isGlissando()) item = spanner;
        }
        if (!item || visited.contains(item) || item->generated()) return;
        visited.insert(item);
        QString identity = itemIdentity(item);
        if (identity.isEmpty()) identity = "source:" + QString::number(visited.size());
        auto measure = score->tick2measure(item->tick());
        QJsonObject attributes {{"sourceType", item->typeName()}, {"visible", item->visible() ? "1" : "0"}};
        if (item->isNote()) {
            auto note = toNote(item);
            attributes["pitch"] = QString::number(note->pitch());
            attributes["play"] = note->play() ? "1" : "0";
            for (Spanner* spanner : note->spannerFor()) collectGlissando(spanner);
        }
        QJsonObject location {
            {"itemIdentity", identity}, {"inventoryOrdinal", int(visited.size() - 1)},
            {"measureId", itemIdentity(measure)},
            {"displayedMeasureNumber", measure ? QString::number(measure->no() + 1) : QString()},
            {"staffId", item->hasStaff() ? QString::number(int(item->staffIdx()) + 1) : QString()},
            {"voice", item->hasStaff() ? QString::number(int(item->voice()) + 1) : QString()},
            {"anchorIds", QJsonArray()}, {"attributes", attributes}
        };
        content.append(location);
        if (!item->visible()) {
            addFinding(inventory.report, "source-visibility", "hidden", "Source visibility fidelity",
                       "source_visibility_requires_correspondence", location, "export", "source_item", false, "unverified", {});
        }
        if (item->isNote() && !toNote(item)->play()) {
            addFinding(inventory.report, "note-playback-disabled", "play-disabled", "Playback-disabled note performance",
                       "muted_note_performance_unverified", location, "playback", "source_item", false, "unverified", {}, {"practice", "game_solo", "flow"});
        }
        if (item->isArpeggio() || item->type() == ElementType::CHORD_BRACKET) {
            auto arpeggio = static_cast<Arpeggio*>(item);
            int subtype = int(arpeggio->arpeggioType());
            QString variant = item->type() == ElementType::CHORD_BRACKET ? "bracket"
                : subtype >= 0 && subtype < variants.size() ? variants[subtype] : "unknown:" + QString::number(subtype);
            auto chord = arpeggio->chord();
            bool expected = item->staff() && item->staff()->show() && item->staff()->part()->show() && chord && chord->visible();
            bool visibleNote = false;
            QJsonArray anchors;
            if (chord) for (const Note* note : chord->notes()) { anchors.append(itemIdentity(note)); visibleNote |= note->visible(); }
            expected &= visibleNote;
            location["anchorIds"] = anchors;
            auto attrs = location["attributes"].toObject(); attrs["subtype"] = QString::number(subtype); location["attributes"] = attrs;
            inventory.items.append(QJsonObject {{"featureId", "arpeggio"}, {"variant", variant}, {"location", location}, {"expectedExport", expected}});
        }
        if (item->isGlissando()) {
            Spanner* glissando = toSpanner(item);
            EngravingItem* start = glissando->startElement();
            EngravingItem* end = glissando->endElement();
            QJsonArray anchors;
            if (start) anchors.append(itemIdentity(start));
            if (end) anchors.append(itemIdentity(end));
            location["anchorIds"] = anchors;
        }
        if (item->isArticulation()) {
            auto articulation = toArticulation(item);
            QString name = QString::fromUtf8(SymNames::nameForSymId(articulation->symId()).ascii());
            auto attrs = location["attributes"].toObject(); attrs["symId"] = name; location["attributes"] = attrs;
            bool core = name.startsWith("articStaccato") || name.startsWith("articStaccatissimoAbove")
                || name.startsWith("articStaccatissimoBelow") || name.startsWith("articTenutoAbove")
                || name.startsWith("articTenutoBelow") || name.startsWith("articAccentAbove")
                || name.startsWith("articAccentBelow") || name.startsWith("articMarcatoAbove") || name.startsWith("articMarcatoBelow")
                || name == "articAccentStaccatoAbove" || name == "articAccentStaccatoBelow"
                || name == "articMarcatoStaccatoAbove" || name == "articMarcatoStaccatoBelow"
                || name == "articMarcatoTenutoAbove" || name == "articMarcatoTenutoBelow"
                || name == "articTenutoAccentAbove" || name == "articTenutoAccentBelow"
                || name == "articTenutoStaccatoAbove" || name == "articTenutoStaccatoBelow";
            QString token;
            QString human;
            if (name.contains("StaccatissimoWedge")) { token = "spicc"; human = "Wedge staccatissimo"; }
            else if (name.contains("StaccatissimoStroke")) token = "stroke";
            else if (name.contains("SoftAccent")) token = "acc-soft";
            else if (name.contains("Unstress")) token = "unstress";
            else if (name.contains("Stress")) token = "stress";
            else if (name.contains("LaissezVibrer")) token = "lv";
            if (!token.isEmpty() && !values(capabilities["supportedArticulationTokens"].toArray()).contains(token)) {
                bool parsed = values(capabilities["parsedArticulationTokens"].toArray()).contains(token);
                addFinding(inventory.report, "articulation-uncommon", token, human.isEmpty() ? "Articulation " + token : human,
                           parsed ? "articulation_render_unverified" : "unsupported_articulation", location,
                           parsed ? "render" : "parse", "source_item", false, parsed ? "unverified" : "omitted",
                           parsed ? QJsonArray() : QJsonArray{"visual"});
            } else if (!core && token.isEmpty()) {
                addFinding(inventory.report, "articulation-uncommon", name, "Articulation variant fidelity", "source_articulation_unverified", location, "render", "source_item", false, "unverified", {});
            }
        }
    };
    score->scanElements(visit);
    // Scan authored tracks explicitly: rendered-page scans omit hidden staves.
    for (Segment* segment = score->firstSegment(SegmentType::All); segment; segment = segment->next1(SegmentType::All)) {
        for (EngravingItem* item : segment->elist()) if (item) item->scanElements(visit);
        for (EngravingItem* item : segment->annotations()) if (item) item->scanElements(visit);
    }
    // Note-anchored spanners do not visit their physical owner through scanElements.
    for (const auto& entry : score->spanner()) collectGlissando(entry.second);
    for (Spanner* glissando : authoredGlissandi) visit(glissando);
    int shown = 0;
    for (Staff* staff : score->staves()) if (staff->show() && staff->part()->show()) ++shown;
    if (shown > 2) {
        QJsonObject location {{"itemIdentity", "source-staff-topology"}, {"anchorIds", QJsonArray()}, {"attributes", QJsonObject{{"visibleStaffCount", QString::number(shown)}}}};
        addFinding(inventory.report, "piano-staff-topology", QString::number(shown), "Piano staff topology",
                   "source_topology_export_unverified", location, "export", "source_item", false, "unverified", {});
    }
    inventory.report["scoreIdentity"] = QFileInfo(sourcePath).fileName() + ":" + sha256(asciiJson(QJsonDocument(content).toJson(QJsonDocument::Compact)));
    finishReport(inventory.report);
    return inventory;
}

QJsonObject mu::project::notationcompatibility::analyzeMei(const QByteArray& mei, const SourceInventory& source,
                        const QHash<QString, QString>& sourceToCanonical, bool canonicalMidiValidated)
{
    QJsonObject report = source.report;
    report["findings"] = QJsonArray();
    report["meiDigest"] = sha256(mei);
    // Exported report identity follows the existing canonical score, not native EID allocation.
    report["scoreIdentity"] = report["meiDigest"].toString();
    const auto publicSourceId = [&sourceToCanonical](const QString& id) {
        return sourceToCanonical.value(QString(id).replace('/', '.').replace('+', '-'));
    };
    const auto publicSourceLocation = [&publicSourceId](QJsonObject location) {
        const QString publicId = publicSourceId(location["itemIdentity"].toString());
        if (!publicId.isEmpty()) location["itemIdentity"] = publicId;
        else if (location.contains("inventoryOrdinal"))
            location["itemIdentity"] = "source:" + QString::number(location["inventoryOrdinal"].toInt());
        const QString measure = publicSourceId(location["measureId"].toString());
        if (!measure.isEmpty()) location["measureId"] = measure;
        else if (!location["measureId"].toString().isEmpty())
            location["measureId"] = "source-measure:" + location["displayedMeasureNumber"].toString();
        QJsonArray anchors;
        for (const QJsonValue& value : location["anchorIds"].toArray()) {
            const QString anchor = publicSourceId(value.toString());
            // Preserve unbound source-anchor presence without inventing a written-note ID.
            anchors.append(anchor.isEmpty() ? "source:" + QString::number(location["inventoryOrdinal"].toInt())
                           + ":anchor:" + QString::number(anchors.size()) : anchor);
        }
        location["anchorIds"] = anchors;
        return location;
    };
    bool inspectedSource = values(source.report["inspectedStages"].toArray()).contains("source");
    report["inspectedStages"] = inspectedSource ? QJsonArray{"source", "export", "parse"} : QJsonArray{"parse"};
    QJsonObject capabilities = catalog();
    QStringList allowed = values(capabilities["acceptedStructuralContainers"].toArray())
        + values(capabilities["acceptedLayerChildren"].toArray())
        + QStringList{"layer", "beam", "tuplet", "graceGrp", "chord"};
    QStringList core = values(capabilities["supportedArticulationTokens"].toArray());
    QList<XmlItem> items;
    QList<int> stack;
    QHash<QString, int> byId;
    QXmlStreamReader reader(mei);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            XmlItem item; item.name = reader.name().toString(); item.namespaceUri = reader.namespaceUri().toString();
            item.parent = stack.isEmpty() ? -1 : stack.last();
            for (const auto& attribute : reader.attributes()) {
                QString key = attribute.namespaceUri().isEmpty() ? attribute.name().toString()
                    : "{" + attribute.namespaceUri().toString() + "}" + attribute.name().toString();
                item.attributes[key] = attribute.value().toString();
            }
            QString id = item.attributes["{http://www.w3.org/XML/1998/namespace}id"].toString();
            item.location = {{"itemIdentity", id.isEmpty() ? "mei:" + QString::number(items.size()) : id},
                             {"inventoryOrdinal", int(items.size())}, {"anchorIds", QJsonArray()}, {"attributes", item.attributes}};
            for (int ancestor = item.parent; ancestor >= 0; ancestor = items[ancestor].parent) {
                if (items[ancestor].name == "measure") {
                    item.location["measureId"] = items[ancestor].attributes["{http://www.w3.org/XML/1998/namespace}id"];
                    item.location["displayedMeasureNumber"] = items[ancestor].attributes["n"];
                }
                if (items[ancestor].name == "staff") item.location["staffId"] = items[ancestor].attributes["n"];
                if (items[ancestor].name == "layer") item.location["voice"] = items[ancestor].attributes["n"];
            }
            QJsonArray anchors;
            for (const QString& name : {QString("startid"), QString("endid"), QString("plist"), QString("{http://pianomania.gg/ns/mei}arpeggio-members")}) {
                for (QString value : item.attributes[name].toString().split(' ', Qt::SkipEmptyParts)) { if (value.startsWith('#')) value.remove(0, 1); anchors.append(value); }
            }
            item.location["anchorIds"] = anchors;
            if (!id.isEmpty()) {
                if (byId.contains(id)) addFinding(report, "note-chord-identity", "duplicate", "Duplicate notation identities", "duplicate_identity", item.location, "parse", "mei_item", true, "", {"structure"});
                byId[id] = items.size();
            }
            items.append(item); stack.append(items.size() - 1);
        } else if (reader.isCharacters() && !stack.isEmpty()) items[stack.last()].text += reader.text().toString();
        else if (reader.isEndElement() && !stack.isEmpty()) stack.removeLast();
    }
    if (reader.hasError()) {
        addFinding(report, "note-chord-identity", "xml", "Malformed MEI", "malformed_xml", {{"itemIdentity", "document"}, {"anchorIds", QJsonArray()}}, "parse", "mei_item", true, "", {"structure"});
    }
    QSet<int> reportedPlayableWrappers;
    for (int index = 0; index < items.size(); ++index) {
        const XmlItem& item = items[index];
        if (item.name == "note") {
            const XmlItem& timing = item.parent >= 0 && items[item.parent].name == "chord" ? items[item.parent] : item;
            bool idxOk = false, beatOk = false, octaveOk = false;
            int idx = item.attributes["idx"].toString().toInt(&idxOk);
            const QString durationText = (item.attributes.contains("dur") ? item.attributes["dur"] : timing.attributes["dur"]).toString();
            const bool durationOk = values(capabilities["acceptedMeiDurations"].toArray()).contains(durationText);
            double beat = timing.attributes["beat"].toString().toDouble(&beatOk);
            int octave = item.attributes["oct"].toString().toInt(&octaveOk);
            QString pitch = item.attributes["pname"].toString();
            bool valid = !item.attributes["{http://www.w3.org/XML/1998/namespace}id"].toString().isEmpty() && idxOk && idx >= 0
                && durationOk && beatOk && std::isfinite(beat) && beat >= 0 && octaveOk && octave >= 0 && octave <= 9
                && pitch.size() == 1 && QString("abcdefg").contains(pitch);
            if (!valid) addFinding(report, "note-chord-identity", "note", "Invalid playable notes", "invalid_note_identity_pitch_timing", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
            for (int ancestor = item.parent; ancestor >= 0; ancestor = items[ancestor].parent) {
                if (!allowed.contains(items[ancestor].name) && !reportedPlayableWrappers.contains(ancestor)) {
                    reportedPlayableWrappers.insert(ancestor);
                    const QString& wrapper = items[ancestor].name;
                    if (canonicalMidiValidated && (wrapper == "bTrem" || wrapper == "fTrem")) {
                        addFinding(report, "tremolo", wrapper, "Tremolo runtime semantics", "tremolo_runtime_unverified",
                                   items[ancestor].location, "model", "mei_item", false, "unverified", {});
                    } else {
                        addFinding(report, "unknown-element-wrapper", wrapper, "Unsupported playable wrappers", "unsupported_playable_wrapper", items[ancestor].location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
                    }
                }
            }
        }
        if ((item.name == "staffDef" || item.name == "staff") && !values(capabilities["topology"].toObject()["currentRuntimeStaffIds"].toArray()).contains(item.attributes["n"].toString())) {
            addFinding(report, "piano-staff-topology", item.attributes["n"].toString(), "Unsupported piano staff topology", "unsupported_staff_identity", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
        }
        if (item.name == "tie") {
            QString start = item.attributes["startid"].toString(); if (start.startsWith('#')) start.remove(0, 1);
            QString end = item.attributes["endid"].toString(); if (end.startsWith('#')) end.remove(0, 1);
            QStringList terminal = item.attributes["{http://pianomania.gg/ns/mei}terminal-ids"].toString().split(' ', Qt::SkipEmptyParts);
            bool terminalValid = !terminal.isEmpty();
            for (QString id : terminal) { if (id.startsWith('#')) id.remove(0, 1); terminalValid &= byId.contains(id); }
            if (start.isEmpty() || !byId.contains(start) || (!end.isEmpty() && !byId.contains(end)) || (end.isEmpty() && !terminalValid)) {
                addFinding(report, "ties-slurs", "tie", "Unresolved ties", "invalid_tie_anchor", item.location, "parse", "mei_item", true, "", {"structure", "playback", "input", "scoring"});
            } else if (end.isEmpty()) {
                addFinding(report, "ties-slurs", "hanging-tie", "Hanging ties", "hanging_tie_visual_omission", item.location, "render", "mei_item");
            }
        }
        for (const QJsonValue& value : capabilities["runtimeOmissions"].toArray()) {
            auto omission = value.toObject();
            if (item.name != omission["element"].toString()) continue;
            addFinding(report, omission["featureId"].toString(), item.name == "arpeg" ? arpeggioVariant(item.attributes) : item.name, omission["humanName"].toString(), "unsupported_notation", item.location, omission["stage"].toString(), "mei_item", false, "", {"visual"}, omission["affectedModes"].toArray());
        }
        if (item.name == "gliss") {
            bool textOmitted = false;
            QString reason = glissandoDisplayOmission(items, index, byId, textOmitted);
            if (reason.isEmpty() && textOmitted) reason = "glissando_text_omission";
            if (!reason.isEmpty()) addFinding(report, "glissando", "gliss", "Glissandos", reason,
                item.location, "render", "mei_item", false,
                reason == "glissando_text_omission" ? "partial" : "omitted", {"visual"});
        }
        if (item.name == "dynam") {
            const QString reason = dynamicVisualIssue(item, capabilities);
            if (!reason.isEmpty()) addFinding(report, "dynamic", "dynam", "Dynamics", reason,
                item.location, "render", "mei_item");
        }
        if (item.name == "arpeg") {
            const QString reason = arpeggioVisualIssue(item.attributes, byId, capabilities);
            if (!reason.isEmpty()) addFinding(report, "arpeggio", arpeggioVariant(item.attributes),
                "Rolled-chord signs", reason, item.location, "render", "mei_item");
        }
        if (QStringList{"fermata", "ornam", "trill", "mordent", "turn", "breath", "caesura"}.contains(item.name)) {
            const QString reason = pianoSymbolVisualIssue(item, capabilities);
            if (!reason.isEmpty()) addFinding(report, "piano-symbol", item.name,
                "Piano notation symbols", reason, item.location, "render", "mei_item");
        }
        if (item.name == "artic") {
            QStringList tokens = item.attributes["artic"].toString().split(' ', Qt::SkipEmptyParts);
            if (!tokens.isEmpty() && std::all_of(tokens.cbegin(), tokens.cend(),
                [&](const QString& token) { return core.contains(token); })) {
                const QString reason = articulationVisualIssue(item, capabilities);
                if (!reason.isEmpty()) addFinding(report, "articulation", tokens.join(' '),
                    "Articulations", reason, item.location, "render", "mei_item");
            }
            if (tokens.size() > 1 && std::any_of(tokens.begin(), tokens.end(), [&](const QString& token) { return !core.contains(token); }))
                addFinding(report, "articulation-combinations", tokens.join(' '), "Combined articulation glyphs", "unsupported_articulation_combination", item.location, "render", "mei_item", false, "partial");
            else if (tokens.size() == 1 && !core.contains(tokens.first())) {
                bool parsed = values(capabilities["parsedArticulationTokens"].toArray()).contains(tokens.first());
                addFinding(report, "articulation-uncommon", tokens.first(), tokens.first() == "spicc" ? "Wedge staccatissimo" : "Articulation " + tokens.first(),
                           parsed ? "articulation_render_unverified" : "unsupported_articulation", item.location,
                           parsed ? "render" : "parse", "mei_item", false, parsed ? "unverified" : "omitted",
                           parsed ? QJsonArray() : QJsonArray{"visual"});
            }
        }
    }
    for (const QJsonValue& value : source.items) {
        auto item = value.toObject(); auto location = item["location"].toObject();
        if (!item["expectedExport"].toBool()) continue;
        QString publicId = sourceToCanonical.value(location["itemIdentity"].toString().replace('/', '.').replace('+', '-'));
        location = publicSourceLocation(location);
        if (!publicId.isEmpty() && byId.contains(publicId)) {
            const auto& exported = items[byId[publicId]];
            if (exported.name == "arpeg" && arpeggioVariant(exported.attributes) != item["variant"].toString())
                addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord signs", "source_variant_missing_from_mei", location, "export", "source_item");
        } else if (!sourceToCanonical.isEmpty()) {
            addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord signs", "source_item_missing_from_mei", location, "export", "source_item");
        } else {
            addFinding(report, "arpeggio", item["variant"].toString(), "Rolled-chord correspondence", "source_variant_correspondence_unverified", location, "export", "source_item", false, "unverified", {});
        }
    }
    // Source property reviews remain uncertain until their exact export policy is verified.
    for (const QJsonValue& value : source.report["findings"].toArray()) {
        auto row = value.toObject();
        if (row["observedBehavior"] == "unverified") {
            QJsonArray locations, examples;
            for (const QJsonValue& value : row["locations"].toArray()) {
                auto location = publicSourceLocation(value.toObject());
                locations.append(location);
                if (examples.size() < 3) examples.append(location["itemIdentity"]);
            }
            row["locations"] = locations;
            row["representativeExamples"] = examples;
            auto findings = report["findings"].toArray(); findings.append(row); report["findings"] = findings;
        }
    }
    auto completeness = report["completeness"].toObject();
    for (const QJsonValue& value : report["targetModes"].toArray()) {
        QString mode = value.toString(); auto stages = completeness[mode].toObject();
        stages["parse"] = admits(report) ? "partial" : "rejected";
        stages["export"] = inspectedSource ? "partial" : "unverified";
        completeness[mode] = stages;
    }
    report["completeness"] = completeness;
    finishReport(report);
    return report;
}

bool mu::project::notationcompatibility::admits(const QJsonObject& report)
{
    if (report["schemaVersion"] != "pianomania.notation-compatibility-report.v1"
        || report["capabilityCatalogDigest"] != NOTATION_CATALOG_DIGEST || !report["findings"].isArray()) return false;
    for (const QJsonValue& row : report["findings"].toArray()) if (row.toObject()["severity"] == "fatal") return false;
    return true;
}
QString mu::project::notationcompatibility::fatalSummary(const QJsonObject& report)
{
    QStringList problems;
    for (const QJsonValue& value : report["findings"].toArray()) {
        auto row = value.toObject();
        if (row["severity"] == "fatal") problems.append(row["humanName"].toString() + ": " + QString::number(row["count"].toInt()));
    }
    return "Score compatibility admission rejected: " + problems.join("; ");
}
SummaryModel::SummaryModel(QObject* parent) : QObject(parent) {}
SummaryModel* SummaryModel::instance()
{
    static SummaryModel* model = new SummaryModel(QCoreApplication::instance());
    return model;
}
QVariantMap SummaryModel::report() const { return m_report.toVariantMap(); }
QString SummaryModel::selectedMode() const { return m_mode; }
QVariantList SummaryModel::findings() const
{
    QVariantList result;
    for (const QJsonValue& value : m_report["findings"].toArray()) {
        auto row = value.toObject();
        if (values(row["affectedModes"].toArray()).contains(m_mode) || row["severity"] == "fatal") result.append(row.toVariantMap());
    }
    return result;
}
QString SummaryModel::summary() const
{
    return admits(m_report) ? QString("Score compatibility: %1 kinds of notation need review").arg(findings().size())
        : QString("Score files rejected: %1 compatibility problems").arg(findings().size());
}
namespace {
struct MusicianIssue {
    QString title;
    QString consequence;
    QString explanation;
    bool fatal = false;
    bool identitiesKnown = true;
    QSet<QString> units;
    QSet<QString> identities;
    QSet<int> measures;
    QSet<int> problemMeasures;
};

QString measureLabel(const QSet<int>& measures)
{
    QList<int> ordered = measures.values();
    std::sort(ordered.begin(), ordered.end());
    QStringList labels;
    for (int index = 0; index < ordered.size();) {
        int first = ordered[index];
        int last = first;
        while (++index < ordered.size() && ordered[index] == last + 1) last = ordered[index];
        labels.append(first == last ? QString("Measure %1").arg(first)
                                    : QString("Measures %1–%2").arg(first).arg(last));
    }
    return labels.join(", ");
}

QString musicalName(const QJsonObject& row)
{
    const QString feature = row["featureId"].toString();
    const QString variant = row["variant"].toString();
    if (feature == "arpeggio") {
        if (variant == "bracket") return "Non-arpeggio brackets";
        if (variant == "up" || variant == "up-straight") return "Upward arpeggios";
        if (variant == "down" || variant == "down-straight") return "Downward arpeggios";
        return "Arpeggios";
    }
    if (feature == "glissando") return "Glissandos";
    if (feature == "harmony") return "Chord symbols";
    if (feature == "lyrics") return "Lyrics";
    if (feature == "figured-bass") return "Figured bass";
    if (feature == "breath-caesura") return variant == "breath" ? "Breath marks" : "Caesuras";
    if (feature == "tremolo" || variant == "bTrem" || variant == "fTrem") return "Tremolos";
    if (feature.startsWith("ornament")) {
        if (variant == "trill") return "Trills";
        if (variant == "mordent") return "Mordents";
        if (variant == "turn") return "Turns";
        return "Ornaments";
    }
    const QMap<QString, QString> articulations {
        {"stacc", "Staccato"}, {"stacciss", "Staccatissimo"}, {"spicc", "Wedge staccatissimo"},
        {"stroke", "Stroke staccatissimo"}, {"ten", "Tenuto"}, {"marc", "Marcato"},
        {"acc", "Accents"}, {"acc-soft", "Soft accents"}, {"stress", "Stress marks"}, {"unstress", "Unstress marks"}
    };
    if (feature == "articulation-combinations") {
        QStringList names;
        for (const QString& token : variant.split(' ', Qt::SkipEmptyParts)) {
            if (!articulations.contains(token)) return "Articulation combinations";
            names.append(articulations[token]);
        }
        return names.isEmpty() ? QString("Articulation combinations") : names.join(" + ");
    }
    if (feature.startsWith("articulation")) return articulations.value(variant, "Articulations");
    if (feature == "piano-staff-topology") return "Staff layout";
    if (feature == "tie" || (feature == "ties-slurs" && row["reasonCode"] == "hanging_tie_visual_omission")) return "Ties";
    return "Score notation";
}

// This presentation-only check mirrors the current arpeggio parser's consumed v1 payload.
// Admission and exact written-note identity validation remain in the machine report.
bool usableArpeggioGeometry(const QJsonObject& row, const QJsonObject& attributes)
{
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    const QString variant = row["variant"].toString();
    if (!QStringList {"normal", "up", "down", "bracket", "up-straight", "down-straight"}.contains(variant)) return false;
    const QString order = attributes["order"].toString();
    if (attributes.contains("order") && !QStringList {"up", "down", "nonarp"}.contains(order)) return false;
    QString style = attributes[prefix + "arpeggio-style"].toString();
    if (attributes.contains(prefix + "arpeggio-style") && !QStringList {"wavy", "straight", "bracket"}.contains(style)) return false;
    if (attributes.contains("lform")) {
        const QString lineForm = attributes["lform"].toString();
        if (lineForm != "wavy" && lineForm != "solid") return false;
        const QString standardStyle = lineForm == "wavy" ? "wavy" : order == "nonarp" ? "bracket" : "straight";
        if (!style.isEmpty() && style != standardStyle) return false;
        style = standardStyle;
    }
    if ((style == "bracket" && order != "nonarp") || (order == "nonarp" && !style.isEmpty() && style != "bracket")) return false;
    const auto validBoolean = [](const QString& value) { return value == "true" || value == "1" || value == "false" || value == "0"; };
    for (const QString& name : {QString("arrow"), prefix + "arpeggio-play"})
        if (attributes.contains(name) && !validBoolean(attributes[name].toString())) return false;
    const auto number = [](const QString& value, double& result) {
        bool ok = false;
        result = value.toDouble(&ok);
        return ok && std::isfinite(result);
    };
    const auto tuple = [&number](const QString& value, int count, QList<double>& result) {
        const QStringList fields = value.split(',');
        if (fields.size() != count) return false;
        result.clear();
        for (const QString& field : fields) {
            double parsed = 0;
            if (!number(field, parsed)) return false;
            result.append(parsed);
        }
        return true;
    };
    QList<double> parsed;
    if (attributes.contains(prefix + "arpeggio-stretch")) {
        double stretch = 0;
        if (!number(attributes[prefix + "arpeggio-stretch"].toString(), stretch) || stretch <= 0) return false;
    }
    if (attributes[prefix + "arpeggio-geometry-version"] != "1") return false;
    const QString pageText = attributes[prefix + "arpeggio-page"].toString();
    bool validPage = false;
    const int page = pageText.toInt(&validPage);
    if (!validPage || page <= 0 || !std::all_of(pageText.cbegin(), pageText.cend(), [](QChar c) { return c >= '0' && c <= '9'; })) return false;
    const QString visibility = attributes[prefix + "arpeggio-visible"].toString();
    if (!validBoolean(visibility)) return false;
    const bool visible = visibility == "true" || visibility == "1";
    if (!tuple(attributes[prefix + "xy"].toString(), 2, parsed)) return false;
    if (!tuple(attributes[prefix + "arpeggio-bounds"].toString(), 4, parsed) || parsed[0] > parsed[2] || parsed[1] < parsed[3]) return false;
    double width = 0;
    if (!number(attributes[prefix + "arpeggio-line-width"].toString(), width) || width <= 0) return false;
    const QString font = attributes[prefix + "arpeggio-font"].toString();
    if ((font != "Leland" && font != "Bravura") || attributes[prefix + "arpeggio-line-cap"] != "flat") return false;
    if (!attributes.contains(prefix + "arpeggio-lines") || !attributes.contains(prefix + "arpeggio-glyphs")) return false;
    int primitives = 0;
    const QString lines = attributes[prefix + "arpeggio-lines"].toString();
    if (!lines.isEmpty()) for (const QString& line : lines.split(';')) {
        if (!tuple(line, 4, parsed) || (parsed[0] == parsed[2] && parsed[1] == parsed[3])) return false;
        ++primitives;
    }
    const QString glyphs = attributes[prefix + "arpeggio-glyphs"].toString();
    if (!glyphs.isEmpty()) for (const QString& glyph : glyphs.split(';')) {
        const QStringList fields = glyph.split(',');
        if (fields.size() != 8 || !QStringList {"wiggleArpeggiatoUp", "wiggleArpeggiatoUpArrow", "arrowheadBlackUp", "arrowheadBlackDown"}.contains(fields[0])) return false;
        if (!tuple(fields.mid(1).join(','), 7, parsed) || parsed[3] >= parsed[5] || parsed[4] <= parsed[6]) return false;
        ++primitives;
    }
    if ((visible && primitives == 0) || (!visible && primitives != 0)) return false;
    // Playing views also require exact notehead registration; incomplete legacy v1 remains a warning.
    if (visible) {
        if (!tuple(attributes[prefix + "arpeggio-anchor-center"].toString(), 2, parsed)) return false;
        const QStringList members = attributes[prefix + "arpeggio-members"].toString().simplified().split(' ', Qt::SkipEmptyParts);
        const QStringList centers = attributes[prefix + "arpeggio-member-centers"].toString().split(';');
        if (members.isEmpty() || centers.size() != members.size()) return false;
        QSet<QString> distinctMembers;
        for (const QString& member : members) {
            if (!member.startsWith('#') || member.size() < 2 || distinctMembers.contains(member)) return false;
            distinctMembers.insert(member);
        }
        for (const QString& center : centers) if (!tuple(center, 2, parsed)) return false;
    }
    return true;
}

// A source-only blanket catalog row cannot establish a concrete visual loss.
bool concreteArpeggioLoss(const QJsonObject& report, const QJsonObject& row, const QJsonObject& location)
{
    if (row["featureId"] != "arpeggio" || row["reasonCode"] != "unsupported_notation") return true;
    for (const QJsonValue& behavior : row["omittedBehavior"].toArray())
        if (behavior != "visual") return true;
    if (row["countUnit"] != "mei_item" || report["meiDigest"].toString().isEmpty()) return false;
    const auto attributes = location["attributes"].toObject();
    const QString prefix = "{http://pianomania.gg/ns/mei}";
    if (attributes[prefix + "arpeggio-visible"] == "false" || attributes[prefix + "arpeggio-visible"] == "0"
        || attributes["visible"] == "false" || attributes["visible"] == "0") return false;
    return !usableArpeggioGeometry(row, attributes);
}

bool lossCoversProduct(const QJsonObject& report, const QJsonObject& row, const QString& behavior)
{
    const QStringList affected = values(row["affectedModes"].toArray());
    bool hasRelevantUse = false;
    for (const QString& use : values(report["targetModes"].toArray())) {
        if ((behavior == "input" || behavior == "scoring") && use.endsWith("_preview")) continue;
        hasRelevantUse = true;
        if (!affected.contains(use)) return false;
    }
    return hasRelevantUse;
}

MusicianIssue musicianIssue(const QJsonObject& report, const QJsonObject& row)
{
    MusicianIssue issue;
    issue.fatal = row["severity"] == "fatal";
    issue.title = musicalName(row);
    const QString reason = row["reasonCode"].toString();
    const QString feature = row["featureId"].toString();
    if (issue.fatal) {
        issue.consequence = reason;
        if (reason == "malformed_xml") {
            issue.title = "Score file";
            issue.explanation = "The score file couldn’t be read. Try exporting it again.";
        } else if (reason == "invalid_tie_anchor") {
            issue.title = "Ties";
            issue.explanation = "Some ties couldn’t be read. Check the affected measures, then retry.";
        } else if (reason == "invalid_note_identity_pitch_timing") {
            issue.title = "Notes and rhythms";
            issue.explanation = "Some notes or rhythms couldn’t be read. Check the affected measures, then retry.";
        } else if (reason == "duplicate_identity") {
            issue.title = "Notes";
            issue.explanation = "Some notes couldn’t be distinguished. Try exporting the score again.";
        } else if (feature == "piano-staff-topology") {
            issue.explanation = "This staff layout isn’t supported yet.";
        } else if (issue.title == "Tremolos") {
            issue.explanation = "Pianomania couldn’t read these tremolos. Check the affected measures, then retry.";
        } else {
            issue.title = "Score compatibility";
            issue.explanation = "Pianomania couldn’t check this score. Try exporting it again.";
        }
        return issue;
    }
    if (reason == "source_variant_missing_from_mei") {
        issue.consequence = "different-marking";
        issue.explanation = "These markings differ from the score in Pianomania.";
        return issue;
    }
    QStringList effects;
    const QJsonArray omitted = row["omittedBehavior"].toArray();
    QString visualSubject = issue.title;
    if (feature == "arpeggio" && row["variant"] != "bracket") visualSubject = "Arpeggio markings";
    else if (feature == "glissando") visualSubject = "Glissando markings";
    else if (feature == "tremolo") visualSubject = "Tremolo markings";
    else if (feature.startsWith("ornament")) visualSubject = "These ornament markings";
    else if (feature.startsWith("articulation")) visualSubject = "These articulation markings";
    if (omitted.contains("visual")) {
        effects.append(feature == "glissando" && reason == "glissando_text_omission"
            ? "Glissando text won’t appear in Pianomania."
            : row["observedBehavior"] == "partial" || !lossCoversProduct(report, row, "visual")
            ? "These markings aren’t shown in full in Pianomania."
            : visualSubject + " won’t appear in Pianomania.");
    }
    if (omitted.contains("playback")) effects.append(lossCoversProduct(report, row, "playback")
        ? "These markings won’t affect playback in Pianomania."
        : "Playback of these markings isn’t fully supported in Pianomania.");
    if (omitted.contains("input")) effects.append(lossCoversProduct(report, row, "input")
        ? "Pianomania won’t check these markings while you play."
        : "Pianomania doesn’t always check these markings while you play.");
    if (omitted.contains("scoring")) effects.append(lossCoversProduct(report, row, "scoring")
        ? "These markings aren’t supported for scoring in Pianomania."
        : "Scoring for these markings isn’t fully supported in Pianomania.");
    if (omitted.contains("traversal")) effects.append(lossCoversProduct(report, row, "traversal")
        ? "Pianomania won’t follow these repeats."
        : "These repeats aren’t fully supported in Pianomania.");
    if (omitted.contains("structure")) effects.append("Pianomania couldn’t read this part of the score.");
    QStringList losses = values(omitted);
    std::sort(losses.begin(), losses.end());
    QStringList coverage;
    for (const QString& loss : losses) coverage.append(loss + (lossCoversProduct(report, row, loss) ? ":all" : ":limited"));
    issue.consequence = row["observedBehavior"].toString() + ":" + coverage.join("|");
    issue.explanation = effects.join(" ");
    return issue;
}
}

bool SummaryModel::needsFix() const { return !admits(m_report); }
QString SummaryModel::musicianTitle() const
{
    return QString("Compatibility warning");
}
QVariantList SummaryModel::musicianFindings() const
{
    // This is a whole-product projection. Internal mode selection remains in the raw findings API only.
    QMap<QString, MusicianIssue> grouped;
    for (const QJsonValue& value : m_report["findings"].toArray()) {
        const auto row = value.toObject();
        const bool fatal = row["severity"] == "fatal";
        const QString observed = row["observedBehavior"].toString();
        if (!fatal && (observed == "unverified" || row["omittedBehavior"].toArray().isEmpty())) continue;
        MusicianIssue issue = musicianIssue(m_report, row);
        if (!fatal && issue.explanation.isEmpty()) continue;
        const QString key = (fatal ? "0:" : "1:") + row["featureId"].toString() + ":"
            + row["variant"].toString() + ":" + issue.consequence;
        const QJsonArray locations = row["locations"].toArray();
        bool hasConcreteIssue = fatal;
        for (const QJsonValue& location : locations)
            if (concreteArpeggioLoss(m_report, row, location.toObject())) hasConcreteIssue = true;
        if (locations.isEmpty() && row["featureId"] != "arpeggio") hasConcreteIssue = true;
        if (!hasConcreteIssue) continue;
        if (!grouped.contains(key)) grouped.insert(key, issue);
        MusicianIssue& target = grouped[key];
        const QString unit = row["countUnit"].toString();
        target.units.insert(unit);
        if (locations.isEmpty()) target.identitiesKnown = false;
        for (const QJsonValue& value : locations) {
            const auto location = value.toObject();
            if (!fatal && !concreteArpeggioLoss(m_report, row, location)) continue;
            const QString identity = location["itemIdentity"].toString();
            if (identity.isEmpty() || identity.startsWith("source:") || identity.startsWith("mei:")
                || (unit != "source_item" && unit != "mei_item")) target.identitiesKnown = false;
            else target.identities.insert(identity);
            bool validNumber = false;
            const int measure = location["displayedMeasureNumber"].toString().toInt(&validNumber);
            if (validNumber && measure >= 0) target.measures.insert(measure);
            const QString measureId = location["measureId"].toString();
            if (m_measureOrdinals.contains(measureId)) target.problemMeasures.insert(m_measureOrdinals.value(measureId));
        }
    }
    if (needsFix() && grouped.isEmpty()) {
        QJsonObject fatal {{"severity", "fatal"}};
        grouped.insert("0:compatibility", musicianIssue(m_report, fatal));
    }
    QVariantList result;
    for (auto iterator = grouped.cbegin(); iterator != grouped.cend(); ++iterator) {
        const MusicianIssue& issue = iterator.value();
        const bool hasCount = !issue.fatal && issue.identitiesKnown && issue.units.size() == 1
            && !issue.identities.isEmpty();
        const int problemMeasure = issue.problemMeasures.isEmpty() ? -1
            : *std::min_element(issue.problemMeasures.cbegin(), issue.problemMeasures.cend());
        const bool canNavigate = problemMeasure >= 0 && m_navigate && m_navigate(problemMeasure, false);
        result.append(QVariantMap {{"title", issue.title}, {"description", issue.explanation},
            {"fatal", issue.fatal}, {"hasCount", hasCount}, {"count", hasCount ? issue.identities.size() : 0},
            {"locationLabel", measureLabel(issue.measures)}, {"problemMeasureOrdinal", problemMeasure}, {"canNavigate", canNavigate}});
    }
    return result;
}

void SummaryModel::setSelectedMode(const QString& mode)
{
    if (mode == m_mode || !values(m_report["targetModes"].toArray()).contains(mode)) return;
    m_mode = mode; emit reportChanged();
}
bool SummaryModel::goToProblem(int findingIndex)
{
    const QVariantList rows = musicianFindings();
    if (!m_navigate || findingIndex < 0 || findingIndex >= rows.size()) return false;
    const QVariantMap row = rows[findingIndex].toMap();
    if (!row["canNavigate"].toBool()) return false;
    return m_navigate(row["problemMeasureOrdinal"].toInt(), true);
}

bool SummaryModel::setReport(const QJsonObject& report, const QHash<QString, int>& measureOrdinals,
                             std::function<bool(int, bool)> navigate)
{
    m_measureOrdinals = measureOrdinals;
    m_navigate = std::move(navigate);
    m_report = report; emit reportChanged();
    QString key = report["scoreIdentity"].toString() + report["sourceDigest"].toString()
        + report["meiDigest"].toString() + report["reportDigest"].toString();
    if (musicianFindings().isEmpty() || m_presented.contains(key)) return false;
    m_presented.insert(key); return true;
}
