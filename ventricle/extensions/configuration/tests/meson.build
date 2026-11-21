identifiers_repository_test = executable(
    'identifiers-repository-test',
    'identifiers_repository_test.cpp',
    dependencies: [configuration_dep, gtest_main_dep],
)
test(
    'identifiers-repository',
    identifiers_repository_test,
    suite: 'configuration',
)

identifier_scopes_test = executable(
    'identifier-scopes-test',
    'identifier_scopes_test.cpp',
    dependencies: [configuration_dep, gtest_main_dep],
)
foreach spec : [
    ['identifier-selection-policy', 'IdentifierSelectionPolicy.*'],
    ['identifier-scopes', 'IdentifierScopesTest.*:Decisions/SelectionDecisionsTest.*'],
]
    test(
        spec[0],
        identifier_scopes_test,
        args: ['--gtest_filter=' + spec[1]],
        suite: 'configuration',
    )
endforeach

schema_probe = executable(
    'configuration-schema-probe',
    'schema_probe.cpp',
    dependencies: configuration_schema_dep,
)
schema_test = executable(
    'configuration-schema-test',
    'schema_test.cpp',
    dependencies: [configuration_schema_dep, gtest_main_dep],
)
test('configuration-schema-generation', schema_test, suite: 'configuration')

if python.found()
    test(
        'configuration-schema',
        python,
        args: [
            '-m',
            'pytest',
            '-c',
            pytest_config,
            '-q',
            meson.current_source_dir() / 'test_schema.py',
        ],
        env: {'SCHEMA_PROBE': schema_probe.full_path()},
        depends: schema_probe,
        suite: ['python', 'configuration'],
    )
endif

model_schema_probe = executable(
    'model-schema-probe',
    'model_schema_probe.cpp',
    dependencies: configuration_schema_dep,
)

if python.found()
    test(
        'model-schema',
        python,
        args: [
            '-m',
            'pytest',
            '-c',
            pytest_config,
            '-q',
            files('test_model_schema.py'),
        ],
        env: {'MODEL_SCHEMA_PROBE': model_schema_probe.full_path()},
        depends: model_schema_probe,
        suite: ['python', 'configuration'],
    )
endif
