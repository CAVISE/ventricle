"""Check generated schemas for scalar attributes and named configuration entries."""

import json
import os
import subprocess

import pytest


def test_attribute_document_matches_schema():
    result = subprocess.run(
        [os.environ['MODEL_SCHEMA_PROBE']],
        capture_output=True,
        text=True,
        check=True,
    )
    schema = json.loads(result.stdout)
    document = {
        'SchemaTest::Sample': {
            'Label': 'hello',
            'Count': 3,
            'Enabled': False,
        }
    }
    assert set(document) <= set(schema['properties'])
    properties = schema['properties']['SchemaTest::Sample']['anyOf'][0]['properties']
    assert set(document['SchemaTest::Sample']) <= set(properties)
    assert properties['Label']['type'] == 'string'
    assert properties['Count']['type'] == 'integer'
    assert properties['Enabled']['type'] == 'boolean'


def test_named_entry_schema():
    result = subprocess.run(
        [os.environ['MODEL_SCHEMA_PROBE']],
        capture_output=True,
        text=True,
        check=True,
    )
    schema = json.loads(result.stdout)
    choices = schema['additionalProperties']['oneOf']
    choice = next(
        item
        for item in choices
        if item['properties']['typeId']['const'] == 'SchemaTest::Sample'
    )
    assert choice['required'] == ['typeId']
    assert choice['additionalProperties'] is False
    assert (
        choice['properties']['overrides']
        == schema['properties']['SchemaTest::Sample']['anyOf'][0]
    )
    assert (
        schema['properties']['SchemaTest::Sample']['anyOf'][1]
        == schema['additionalProperties']
    )


HOLDERS = (
    'devices',
    'channels',
    'objects',
    'mobilityModels',
    'networkStacks',
    'applications',
    'nodePolicies',
    'nodes',
    'defaults',
)


@pytest.mark.parametrize('name', HOLDERS)
def test_holder_schema(name):
    result = subprocess.run(
        [os.environ['MODEL_SCHEMA_PROBE']],
        capture_output=True,
        text=True,
        check=True,
    )
    schema = json.loads(result.stdout)
    holder = schema['properties'][name]
    assert holder['type'] == 'object'
    assert set(HOLDERS).isdisjoint(holder['properties'])
    assert (
        holder['properties']['SchemaTest::Sample']
        == schema['properties']['SchemaTest::Sample']
    )
    assert holder['additionalProperties'] == schema['additionalProperties']
