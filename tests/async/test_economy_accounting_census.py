"""Verify the real source census and classified writer inventory."""

import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "economy_accounting", ROOT / "scripts/validate_economy_accounting.py"
)
CONTRACT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CONTRACT)


class EconomyAccountingCensusTest(unittest.TestCase):
    def test_real_census_inventory_and_golden_fixtures(self):
        folder = ROOT / "docs/persistence/economy_accounting"
        registry = json.loads((folder / "registry.json").read_text(encoding="utf-8"))
        golden = json.loads((folder / "golden.json").read_text(encoding="utf-8"))
        inventory = json.loads((folder / "writers.json").read_text(encoding="utf-8"))

        CONTRACT.validate_registry(registry)
        scanned = CONTRACT.scan_sources(ROOT)
        reward_item_sql = {
            (row["path"], row["line"], row["family"])
            for row in scanned
            if row["family"] == "sql_reward_items"
        }
        self.assertEqual(
            reward_item_sql,
            {
                ("src/account/account_reward.c", 452, "sql_reward_items"),
                ("src/account/account_reward.c", 457, "sql_reward_items"),
                ("src/account/account_reward.c", 461, "sql_reward_items"),
                ("src/account/account_reward.c", 464, "sql_reward_items"),
            },
        )
        CONTRACT.unique(golden["fixtures"], "id", "fixture ID")
        for fixture in golden["fixtures"]:
            CONTRACT.validate_fixture(fixture, registry)
        if inventory.get("census_complete"):
            CONTRACT.validate_inventory(inventory, registry, ROOT, census=True)
        else:
            CONTRACT.validate_inventory(inventory, registry, ROOT)
            census = {(row["path"], row["line"], row["family"]) for row in inventory["census"]}
            mapped = {tuple(site) for writer in inventory["writers"] for site in writer.get("sites", [])}
            reviewed = set()
            for review in inventory.get("nonwriters", []):
                if review["classification"] == "temporary_inspection":
                    reviewed.update(tuple(site) for site in review["sites"])
                else:
                    reviewed.add(tuple(review["site"]))
            self.assertEqual(census, mapped | reviewed)


if __name__ == "__main__":
    unittest.main()
