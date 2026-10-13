#!/usr/bin/env python3
"""Source-only regression checks for the effective authoring policy registry."""
import argparse
import collections
import importlib.util
import json
from pathlib import Path
import re
import sys
import unittest
import xml.etree.ElementTree as ET

spec = importlib.util.spec_from_file_location("authoring_coverage", Path(__file__).with_name("authoring-policy-coverage.py"))
coverage = importlib.util.module_from_spec(spec)
spec.loader.exec_module(coverage)


class RegistryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.original = json.loads(INVENTORY.read_text())["rows"]
        cls.rows = coverage.load_rows(INVENTORY)
        cls.ranges = json.loads((SOURCE / "fonts/smufl/ranges.json").read_text())
        cls.header = coverage.generate(cls.rows, cls.ranges).decode()

    def test_effective_row_counts_and_exact_override_set(self):
        self.assertEqual(collections.Counter(r["recommendation"] for r in self.rows), {"hide creation": 1675, "retain pending review": 2356, "keep": 1100})
        original = {r["feature_id"]: r["recommendation"] for r in self.original}
        changed = {r["feature_id"]: r["recommendation"] for r in self.rows if original[r["feature_id"]] != r["recommendation"]}
        self.assertEqual(changed, coverage.OVERRIDES)

    def test_all_original_pending_rows_remain_pending(self):
        effective = {r["feature_id"]: r["recommendation"] for r in self.rows}
        pending = [r for r in self.original if r["recommendation"] == "retain pending review"]
        self.assertEqual(len(pending), 2355)
        for row in pending:
            self.assertEqual(effective[row["feature_id"]], "retain pending review")

    def test_ambiguous_semantics_are_not_globally_hidden(self):
        body = re.search(r"hiddenPlayingTechniques = \{\{(.*?)\}\};", self.header, re.S).group(1)
        for retained in ["Distortion", "Natural", "Harmonics", "JazzTone"]:
            self.assertNotIn("PlayingTechniqueType::" + retained, body)
        body = re.search(r"hiddenArticulationSymbols = \{\{(.*?)\}\};", self.header, re.S).group(1)
        self.assertNotIn("SymId::stringsThumbPosition", body)

    def test_exact_symbol_range_union(self):
        hidden = [r for r in self.rows if r["recommendation"] == "hide creation"]
        symbols = {r["variant"] for r in hidden if r["family"] == "master_symbol_variant"}
        ranges = [r["variant"] for r in hidden if r["family"] == "master_symbol_range"]
        self.assertEqual(len(symbols), 751)
        self.assertEqual(len(ranges), 30)
        self.assertEqual(symbols, {g for key in ranges for g in self.ranges[key]["glyphs"]})

    def test_current_instrument_ids_exist_without_rewriting_repository(self):
        ids = {e.get("id") for e in ET.parse(SOURCE / "share/instruments/instruments.xml").iter("Instrument")}
        all_instruments = [r for r in self.rows if r["family"] == "instrument"]
        self.assertEqual(len(all_instruments), 644)
        self.assertTrue(all(r["variant"] in ids for r in all_instruments))
        self.assertEqual(sum(r["recommendation"] == "hide creation" for r in all_instruments), 637)

    def test_bundled_templates_exist_and_user_template_is_pending(self):
        hidden = [r for r in self.rows if r["family"] == "template" and r["recommendation"] == "hide creation"]
        self.assertEqual(len(hidden), 37)
        self.assertTrue(all((SOURCE / "share/templates" / r["variant"]).is_file() for r in hidden))
        user = next(r for r in self.rows if r["feature_id"] == "template.custom.user")
        self.assertEqual(user["recommendation"], "retain pending review")

    def test_header_is_deterministic_and_matches_source(self):
        self.assertEqual(self.header.encode(), coverage.generate(self.rows, self.ranges))
        self.assertEqual(self.header, (SOURCE / "src/notation/internal/authoringcapabilitypolicydata.h").read_text())

    def test_unapproved_inventory_input_fails(self):
        class ChangedInput:
            def read_bytes(self):
                return b'{"rows": []}'
        with self.assertRaisesRegex(ValueError, "digest changed"):
            coverage.load_rows(ChangedInput())

    def test_effective_route_obligations_remain_explicit(self):
        self.assertEqual(coverage.coverage(self.rows)["effective_hidden_recorded_obligations"], 5057)
        for row in self.rows:
            if row["recommendation"] == "hide creation":
                self.assertTrue(row["authoring_surfaces"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    INVENTORY, SOURCE = args.inventory, args.source
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(RegistryTests))
    if args.receipt:
        args.receipt.write_text(json.dumps({"tests": result.testsRun, "passed": result.testsRun - len(result.failures) - len(result.errors), "failures": len(result.failures), "errors": len(result.errors), "boundary": "Source registry/generator checks only; native compilation and behavior not run."}, indent=2) + "\n")
    sys.exit(not result.wasSuccessful())
