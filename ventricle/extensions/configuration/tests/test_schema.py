"""Check inherited scalar attributes and schema input validation."""

import json
import os
import subprocess

import pytest


def generate(mode='valid'):
    return subprocess.run(
        [os.environ['SCHEMA_PROBE'], mode], capture_output=True, text=True
    )


def test_inherited_construction_attributes():
    result = generate()
    assert result.returncode == 0, result.stderr
    schema = json.loads(result.stdout)
    derived = schema['properties']['SchemaTest::Derived']['anyOf'][0]
    assert derived['additionalProperties'] is False
    assert not derived.get('required')
    assert derived['properties']['Label']['type'] == 'string'
    assert derived['properties']['Label']['default'] == 'default'
    assert derived['properties']['Enabled']['type'] == 'boolean'
    assert derived['properties']['Enabled']['default'] is True


@pytest.mark.parametrize(
    ('mode', 'message'),
    [
        ('duplicate', 'Duplicate exposed TypeId'),
        ('invalid', 'uninitialized TypeId'),
    ],
)
def test_invalid_types(mode, message):
    result = generate(mode)
    assert result.returncode == 1
    assert message in result.stderr
