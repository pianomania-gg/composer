# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set(MODULE_TEST iex_mei_tests)

set(MODULE_TEST_SRC
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.cpp
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.h
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorecomp.cpp
    ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorecomp.h

    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mei_tests.cpp
)

set(MODULE_TEST_LINK
    engraving
    iex_mei
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

string(CONCAT PIANOMANIA_MEI_TEST_FILTER
    "Mei_Tests.mei_ending_label_exports_plain_display_text:"
    "Mei_Tests.mei_hairpin_export_includes_pm_hairpin_lines_when_endpoints_present:"
    "Mei_Tests.mei_line_hairpin_exports_resolved_center_and_layout_y_offset:"
    "Mei_Tests.mei_directives_export_resolved_center_between_staves:"
    "Mei_Tests.mei_export_omits_invisible_note_and_idx:"
    "Mei_Tests.mei_export_rehearsal_mark_on_hidden_rest:"
    "Mei_Tests.mei_pianomania_grace_same_pitch_indices:"
    "Mei_Tests.pianomania_text_directive_exports_text_box_centre:"
    "Mei_Tests.mei_export_laissez_vibrer_keeps_pm_geometry:"
    "Mei_Tests.mei_export_final_barline_pedal_keeps_rendered_geometry_without_endid:"
    "Mei_Tests.mei_export_connected_pedal_includes_owned_rendered_segments:"
    "Mei_Tests.mei_export_pianomania_rubato_zone:"
    "Mei_Tests.mei_rubato_overlay_preserves_layout_and_export_when_hidden:"
    "Mei_Tests.mei_hidden_tempo_preserves_text_and_playback_with_visibility:"
    "Mei_Tests.mei_chopin_rubato_overlay_does_not_move_notation:"
    "Mei_Tests.mei_export_pianomania_470_properties_survive_mscx_round_trip:"
    "Mei_Tests.mei_export_pianomania_rubato_zone_resolves_hidden_boundaries_to_one_anchor:"
    "Mei_Tests.mei_export_beam_and_tuplet_boundaries_ignore_omitted_hidden_chord:"
    "Mei_Tests.mei_export_tuplets_include_complete_resolved_geometry:"
    "Mei_Tests.mei_export_hidden_tuplet_semantic_visibility_matches_drawn_geometry:"
    "Mei_Tests.mei_export_hairpins_ending_without_onset_bind_end_element:"
    "Mei_Tests.mei_export_omits_spanner_segments_without_a_score_page:"
    "Mei_Tests.mei_export_spanners_on_hidden_rests_use_visible_same_staff_notes:"
    "Mei_Tests.mei_export_pianomania_rubato_zone_overlap_fails:"
    "Mei_Tests.mei_export_pianomania_pyro_span_free_range:"
    "Mei_Tests.mei_export_pianomania_laser_span_coexists_with_rubato_zone:"
    "Mei_Tests.mei_export_pianomania_pyro_span_overlap_fails:"
    "Mei_Tests.mei_export_articulation_centres_match_native_rendered_bounds:"
    "Mei_Tests.mei_export_articulation_centres_preserve_native_notehead_anchors:"
    "Mei_Tests.mei_export_wedge_keeps_only_owned_endpoint_system_geometry:"
    "Mei_Tests.pianomania_staff_centered_dynamic_clears_ledger_accidental:"
    "Mei_Tests.mei_export_trill_and_ornaments_carry_drawn_glyph_geometry:"
    "Mei_Tests.mei_export_small_notes_chords_and_rests_are_cue:"
    "Mei_Tests.mei_export_key_signature_symbols_match_layout:"
    "Mei_Tests.mei_export_hidden_accidental_is_gestural_only:"
    "Mei_Tests.mei_export_header_clef_change_at_system_start:"
    "Mei_Tests.pianomania_cross_staff_rest_clears_destination_chord:"
    "Mei_Tests.pianomania_staff_centered_dynamic_keeps_cross_staff_beam_margin:"
    "Mei_Tests.mei_export_snapped_expression_carries_text_origin"
)

add_test(
    NAME iex_mei_pianomania_tests
    COMMAND iex_mei_tests
        --gtest_filter=${PIANOMANIA_MEI_TEST_FILTER}
)

# GTest exits 0 when a filter matches nothing, so a gate whose tests were dropped
# would report success while running zero tests. Require the exact expected count.
# runutests.sh sets GTEST_COLOR=1, so a colour escape sits between the tag and the
# count. Skip whatever separates them instead of assuming a single space.
set_tests_properties(iex_mei_pianomania_tests PROPERTIES
    PASS_REGULAR_EXPRESSION "\\[  PASSED  \\][^0-9]*39 tests"
)
