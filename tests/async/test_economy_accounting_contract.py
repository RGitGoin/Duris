#!/usr/bin/env python3
"""Negative and behavioral checks for the backend-neutral accounting contract."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest
import tempfile

ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('accounting_contract',ROOT/'scripts/validate_economy_accounting.py')
contract=importlib.util.module_from_spec(spec)
spec.loader.exec_module(contract)
FOLDER=ROOT/'docs/persistence/economy_accounting'

class AccountingContractTest(unittest.TestCase):
    def setUp(self):
        self.registry=json.loads((FOLDER/'registry.json').read_text())
        self.examples={f['id']:f for f in json.loads((FOLDER/'golden.json').read_text())['fixtures']}

    def validate(self,name):
        return contract.validate_fixture(self.examples[name],self.registry)

    def test_single_item_decoder_is_a_lifecycle_candidate(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            (root/'src').mkdir()
            (root/'src/probe.c').write_text(
                'P_obj restored = read_one_object(blob);\n'
                '// read_one_object(comment);\n'
                'const char *label = "read_one_object(string)";\n')
            sites=contract.scan_sources(root)
            self.assertEqual([(s['line'],s['family']) for s in sites],
                             [(1,'item_lifecycle')])

    def test_all_golden_examples(self):
        contract.validate_registry(self.registry)
        for name in self.examples:
            with self.subTest(name=name): self.validate(name)

    def test_wallet_bank_exact_denominations(self):
        holdings,_=self.validate('wallet_bank')
        self.assertEqual(holdings['wallet']['balance'],[0,1,5,0])
        self.assertEqual(holdings['bank']['balance'],[0,0,5,0])

    def test_staged_settlement_does_not_debit_buyer_twice(self):
        holdings,custody=self.validate('auction_staged')
        self.assertEqual(contract.copper(holdings['buyer']['balance']),2000)
        self.assertEqual(contract.copper(holdings['seller']['balance']),9500)
        self.assertEqual(contract.copper(holdings['escrow']['balance']),0)
        self.assertEqual(contract.copper(holdings['claim']['balance']),0)
        self.assertEqual(custody['701']['identity'],2)

    def test_exact_replay_does_not_repeat_effects(self):
        before=self.validate('expense')
        f=self.examples['expense'];f['operations'].append(copy.deepcopy(f['operations'][0]))
        self.assertEqual(self.validate('expense'),before)

    def test_changed_replay_payload_is_conflict(self):
        f=self.examples['expense'];changed=copy.deepcopy(f['operations'][0]);changed['actor']='operator'
        f['operations'].append(changed)
        with self.assertRaisesRegex(contract.ContractError,'payload conflict'): self.validate('expense')

    def test_fresh_operation_cannot_repeat_reward_source(self):
        f=self.examples['reward'];changed=copy.deepcopy(f['operations'][0]);changed['operation_id']='aa'*16
        f['operations'].append(changed)
        with self.assertRaisesRegex(contract.ContractError,'duplicate source'): self.validate('reward')

    def test_missing_source(self):
        self.examples['reward']['operations'][0]['source_event']=None
        with self.assertRaisesRegex(contract.ContractError,'source event'): self.validate('reward')

    def test_unknown_reason(self):
        self.examples['expense']['operations'][0]['reason']='arbitrary_balance'
        with self.assertRaisesRegex(contract.ContractError,'unknown reason'): self.validate('expense')

    def test_wrong_issuance_sign(self):
        for p in self.examples['reward']['operations'][0]['postings']: p['delta']=[-n for n in p['delta']]
        with self.assertRaisesRegex(contract.ContractError,'issuance sign'): self.validate('reward')

    def test_unbalanced(self):
        self.examples['expense']['operations'][0]['postings'][-1]['delta'][0]+=1
        with self.assertRaisesRegex(contract.ContractError,'unbalanced'): self.validate('expense')

    def test_scalar_balance_does_not_excuse_negative_denomination(self):
        self.examples['expense']['operations'][0]['postings'][0]['delta']=[-5,-2,-1,0]
        with self.assertRaisesRegex(contract.ContractError,'negative holding'): self.validate('expense')

    def test_copper_overflow(self):
        with self.assertRaisesRegex(contract.ContractError,'copper overflow'): contract.copper([0,0,0,2**63-1])
        with self.assertRaisesRegex(contract.ContractError,'denomination overflow'): contract.copper([True,0,0,0])

    def test_duplicate_account_identity(self):
        f=self.examples['wallet_bank'];f['holdings']['alias']=copy.deepcopy(f['holdings']['wallet'])
        with self.assertRaisesRegex(contract.ContractError,'duplicate account'): self.validate('wallet_bank')

    def test_named_ids_reject_missing_blank_and_nonstring_values(self):
        for label in ('writer ID','fixture ID','account_kinds','reasons'):
            for row in ({},{'id':''},{'id':'  '},{'id':None},{'id':7},{'id':[]}):
                with self.subTest(label=label,row=row):
                    with self.assertRaises(contract.ContractError):
                        contract.unique([row],'id',label)

    def test_registry_rejects_blank_named_id_before_references(self):
        for section in ('account_kinds','reasons'):
            changed=copy.deepcopy(self.registry)
            changed[section][0]['id']=' '
            with self.subTest(section=section):
                with self.assertRaisesRegex(contract.ContractError,'invalid '+section):
                    contract.validate_registry(changed)

    def test_duplicate_registry_id(self):
        self.registry['reasons'].append(copy.deepcopy(self.registry['reasons'][0]))
        with self.assertRaisesRegex(contract.ContractError,'duplicate'): contract.validate_registry(self.registry)

    def test_stale_item_before(self):
        self.examples['same_owner_container_move']['operations'][0]['items'][0]['before']['identity']=99
        with self.assertRaisesRegex(contract.ContractError,'stale item'): self.validate('same_owner_container_move')

    def test_cycle_rejected(self):
        f=self.examples['same_owner_container_move'];f['custody']['501']['parent']=502
        f['custody']['502']['root']=501;f['custody']['502']['parent']=501
        with self.assertRaisesRegex(contract.ContractError,'cyclic'): self.validate('same_owner_container_move')

    def test_containment_cannot_cross_owner_context(self):
        custody = {
            '1': dict(kind='pet', identity=7, context=100, root=1, parent=0),
            '2': dict(kind='pet', identity=7, context=200, root=1, parent=1),
        }
        with self.assertRaisesRegex(contract.ContractError, 'inconsistent owner/root'):
            contract.topology(custody)
        custody['2']['context'] = 100
        contract.topology(custody)

    def test_custody_context_is_bounded_and_defaults_to_zero(self):
        custody = {'1': dict(kind='player', identity=7, root=1, parent=0)}
        contract.topology(custody)
        for context in (-1, 2**64, True, '0'):
            with self.subTest(context=context):
                custody['1']['context'] = context
                with self.assertRaisesRegex(contract.ContractError, 'invalid custody context'):
                    contract.topology(custody)

    def test_multiple_ordered_events_for_same_item(self):
        f=self.examples['same_owner_container_move'];op=f['operations'][0];event=op['items'][0]
        second=copy.deepcopy(event);second.update(event_index=1,before=copy.deepcopy(event['after']),after=copy.deepcopy(event['before']))
        op['items'].append(second)
        _,custody=self.validate('same_owner_container_move')
        self.assertEqual(custody['502'],event['before'])

    def test_unlinked_item_event(self):
        self.examples['same_owner_container_move']['operations'][0]['items'][0]['operation_id']='aa'*16
        with self.assertRaisesRegex(contract.ContractError,'unlinked item'): self.validate('same_owner_container_move')

    def test_duplicate_item_event_index(self):
        f=self.examples['same_owner_container_move'];f['operations'][0]['items']*=2
        with self.assertRaisesRegex(contract.ContractError,'duplicate item event'): self.validate('same_owner_container_move')

    def test_intent_cannot_reduce_existing_payload_capacity(self):
        self.registry['limits']['intent_bytes']=16384
        with self.assertRaisesRegex(contract.ContractError,'intent'): contract.validate_registry(self.registry)

    def test_inventory_rejects_missing_backend_and_dependency(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text('void example() {}\n')
            (root/'test.py').write_text('print("synthetic fixture")\n')
            writer=dict(id='example',reason='bank_transfer',owner='test',authority_boundary='test',
                classification='transfer',integration_issue=480,coverage='legacy',path='src/example.c',
                symbol='example',test_candidates=['test.py'],
                backends={name:dict(status='unverified') for name in ('mysql','mariadb','flatfile')})
            inventory=dict(schema_version=1,writers=[writer],census=[],census_complete=False)
            contract.validate_inventory(inventory,self.registry,root)
            missing=copy.deepcopy(inventory);del missing['writers'][0]['backends']['flatfile']
            with self.assertRaisesRegex(contract.ContractError,'missing backend'):
                contract.validate_inventory(missing,self.registry,root)
            writer['integration_issue']=491
            with self.assertRaisesRegex(contract.ContractError,'slice dependency'):
                contract.validate_inventory(inventory,self.registry,root)

    def test_inventory_rejects_stale_source_coordinates(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text('void example() { ch->points.cash[index] += amount; }\n')
            writer=dict(id='example',reason='bank_transfer',owner='test',authority_boundary='test',
                classification='transfer',integration_issue=480,coverage='legacy',path='src/example.c',
                symbol='example',test_candidates=[],sites=[['src/example.c',1,'direct_cash_assignment']],
                backends={name:dict(status='unverified') for name in ('mysql','mariadb','flatfile')})
            inventory=dict(schema_version=1,writers=[writer],census=contract.scan_sources(root))
            contract.validate_inventory(inventory,self.registry,root)
            for field,value in ((0,'src/missing.c'),(1,2),(2,'item_lifecycle')):
                stale=copy.deepcopy(inventory);stale['writers'][0]['sites'][0][field]=value
                with self.subTest(field=field), self.assertRaisesRegex(contract.ContractError,'source site'):
                    contract.validate_inventory(stale,self.registry,root)

    def census_fixture(self, root):
        (root/'src').mkdir()
        (root/'src/example.c').write_text('void example() { ch->points.cash[index] += amount; }\n')
        (root/'test.py').write_text('print("synthetic fixture")\n')
        writer=dict(id='example',reason='bank_transfer',owner='test',authority_boundary='test',
            classification='transfer',integration_issue=480,coverage='legacy',path='src/example.c',
            symbol='example',source='wallet',destination='bank',test_candidates=['test.py'],sites=[['src/example.c',1,'direct_cash_assignment']],
            backends={name:dict(status='unverified') for name in ('mysql','mariadb','flatfile')})
        return dict(schema_version=1,writers=[writer],census=contract.scan_sources(root),census_complete=True)

    def test_complete_census_does_not_require_gameplay_enforcement(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.census_fixture(root)
            contract.validate_inventory(inventory,self.registry,root,census=True)
            with self.assertRaisesRegex(contract.ContractError,'executable evidence'):
                contract.validate_inventory(inventory,self.registry,root,release=True)

    def test_complete_census_requires_transfer_and_test_metadata(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.census_fixture(root)
            for field,value in (('source',None),('source',' '),('destination',''),
                                ('destination',42),('test_candidates',[])):
                changed=copy.deepcopy(inventory)
                changed['writers'][0][field]=value
                with self.subTest(field=field,value=value):
                    with self.assertRaisesRegex(contract.ContractError,'missing'):
                        contract.validate_inventory(changed,self.registry,root,census=True)
                    changed['census_complete']=False
                    contract.validate_inventory(changed,self.registry,root)

    def test_census_gate_refuses_unreviewed_inventory(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.census_fixture(root)
            inventory['census_complete']=False
            contract.validate_inventory(inventory,self.registry,root)
            with self.assertRaisesRegex(contract.ContractError,'writer census not complete'):
                contract.validate_inventory(inventory,self.registry,root,census=True)

    def test_complete_claim_cannot_hide_unmapped_candidate(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.census_fixture(root)
            inventory['writers'][0]['sites']=[]
            for strict in (False,True):
                with self.subTest(census=strict), self.assertRaisesRegex(contract.ContractError,'unclassified writer candidate'):
                    contract.validate_inventory(inventory,self.registry,root,census=strict)

    def declaration_fixture(self, root):
        inventory=self.census_fixture(root)
        (root/'src/example.c').write_text('bool currency_transaction_submit(\n    int amount);\n')
        inventory['census']=contract.scan_sources(root)
        inventory['writers']=[]
        fragment='bool currency_transaction_submit(\n    int amount);'
        inventory['nonwriters']=[dict(site=['src/example.c',1,'economic_submit'],
            classification='declaration',rationale='Prototype only',end_line=2,
            source_sha256=contract.hashlib.sha256(fragment.encode()).hexdigest())]
        return inventory

    def test_reviewed_declarations_can_complete_census(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.declaration_fixture(root)
            contract.validate_inventory(inventory,self.registry,root,census=True)

    def test_reviewed_extern_declaration_can_complete_census(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.declaration_fixture(root)
            fragment='extern bool currency_transaction_submit(int amount);'
            (root/'src/example.c').write_text(fragment+'\n')
            inventory['census']=contract.scan_sources(root)
            inventory['nonwriters'][0].update(end_line=1,
                source_sha256=contract.hashlib.sha256(fragment.encode()).hexdigest())
            contract.validate_inventory(inventory,self.registry,root,census=True)
            fragment='extern bool currency_transaction_submit() { return true; };'
            (root/'src/example.c').write_text(fragment+'\n')
            inventory['census']=contract.scan_sources(root)
            inventory['nonwriters'][0]['source_sha256']=contract.hashlib.sha256(fragment.encode()).hexdigest()
            with self.assertRaisesRegex(contract.ContractError,'not a reviewed declaration'):
                contract.validate_inventory(inventory,self.registry,root,census=True)

    def test_declaration_review_detects_change_after_first_line(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.declaration_fixture(root)
            (root/'src/example.c').write_text('bool currency_transaction_submit(\n    long amount);\n')
            self.assertEqual(contract.scan_sources(root),inventory['census'])
            with self.assertRaisesRegex(contract.ContractError,'declaration changed'):
                contract.validate_inventory(inventory,self.registry,root)

    def test_nonwriter_requires_unique_current_site_and_rationale(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.declaration_fixture(root)
            cases=[('rationale',' ','rationale'),('classification','projection','classification'),
                   ('site',['src/example.c',2,'economic_submit'],'source site')]
            for key,value,error in cases:
                changed=copy.deepcopy(inventory);changed['nonwriters'][0][key]=value
                with self.subTest(key=key), self.assertRaisesRegex(contract.ContractError,error):
                    contract.validate_inventory(changed,self.registry,root)
            inventory['nonwriters']*=2
            with self.assertRaisesRegex(contract.ContractError,'conflicting'):
                contract.validate_inventory(inventory,self.registry,root)

    def test_writer_cannot_also_be_excluded(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.census_fixture(root)
            inventory['nonwriters']=[dict(site=inventory['writers'][0]['sites'][0])]
            with self.assertRaisesRegex(contract.ContractError,'conflicting'):
                contract.validate_inventory(inventory,self.registry,root)

    def test_function_body_cannot_be_excluded_as_declaration(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);inventory=self.declaration_fixture(root)
            fragment='bool currency_transaction_submit() { return true; };'
            (root/'src/example.c').write_text(fragment+'\n')
            inventory['census']=contract.scan_sources(root)
            inventory['nonwriters'][0].update(end_line=1,
                source_sha256=contract.hashlib.sha256(fragment.encode()).hexdigest())
            with self.assertRaisesRegex(contract.ContractError,'not a reviewed declaration'):
                contract.validate_inventory(inventory,self.registry,root)

    def test_unproven_projection_cannot_bypass_release_gate(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir();(root/'src/example.c').write_text('void example() {}\n')
            writer=dict(id='example',reason='item_move',owner='test',authority_boundary='test',
                classification='hydration',integration_issue=486,coverage='projection',path='src/example.c',
                symbol='example',test_candidates=[],evidence=[],
                backends={name:dict(status='projection') for name in ('mysql','mariadb','flatfile')})
            inventory=dict(schema_version=1,writers=[writer],census=[],census_complete=True)
            self.registry['status']='frozen'
            with self.assertRaisesRegex(contract.ContractError,'executable evidence'):
                contract.validate_inventory(inventory,self.registry,root,release=True)

    def test_sql_census_is_case_insensitive_but_ignores_comments(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text(
                'query("insert ignore into account_banks values (1)");\n'
                'query("UpDaTe account_banks set bank_copper=1");\n'
                'query("delete from saved_items where id=1");\n'
                '// update account_banks set bank_copper=2\n'
                '/* INSERT INTO saved_items values (2) */\n'
                'add_money(ch, 1);\n')
            self.assertEqual([(s['line'],s['family']) for s in contract.scan_sources(root)],
                [(1,'sql_economy'),(2,'sql_economy'),(3,'sql_economy')])

    def test_census_tracks_indirect_inventory_consumption(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text(
                'void vnum_from_inv(P_char ch, int vnum, int count);\n'
                'void consume() { vnum_from_inv(ch, material, 2); }\n'
                '// vnum_from_inv(ch, material, 1);\n'
                'const char *text = "vnum_from_inv(ch, material, 1)";\n'
                'void inspect() { vnum_in_inv(ch, material); }\n')
            self.assertEqual([(s['line'],s['family']) for s in contract.scan_sources(root)],
                [(1,'item_lifecycle'),(2,'item_lifecycle')])

    def test_census_tracks_order_preserving_container_publication(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text(
                'void restore() { obj_to_obj_at_end(child, parent); }\n'
                'void normal() { obj_to_obj(child, parent); }\n'
                '// obj_to_obj_at_end(child, parent);\n')
            self.assertEqual([(s['line'],s['family']) for s in contract.scan_sources(root)],
                [(1,'item_publication'),(2,'item_publication')])

    def test_census_tracks_clear_money_callers_and_macro_body(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text(
                '#define CLEAR_MONEY(ch) GET_COPPER(ch) = 0;\n'
                'void example() { CLEAR_MONEY(ch); }\n'
                '// CLEAR_MONEY(ch);\n'
                'const char *message = "CLEAR_MONEY(ch)";\n')
            sites=contract.scan_sources(root)
            self.assertEqual({(s['line'],s['family']) for s in sites},
                {(1,'money_helper'),(1,'coin_assignment'),(2,'money_helper')})

    def test_census_detects_direct_cash_and_ignores_comments(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src').mkdir()
            (root/'src/example.c').write_text('void example() { ch->points.cash[index] += amount; }\n// ADD_MONEY(ch,1,0,0,0);\n')
            sites=contract.scan_sources(root)
            self.assertEqual([(s['line'],s['family']) for s in sites],[(1,'direct_cash_assignment')])

if __name__=='__main__': unittest.main()
