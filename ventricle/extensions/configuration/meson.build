configuration_identifier_values = {
    'REFERENCE': 'x-ventricle-reference',
    'TYPE_ID': 'x-ventricle-type',
    'OVERRIDES': 'overrides',
    'DEVICES': 'devices',
    'CHANNELS': 'channels',
    'OBJECTS': 'objects',
    'MOBILITY_MODELS': 'mobilityModels',
    'NETWORK_STACKS': 'networkStacks',
    'APPLICATIONS': 'applications',
    'NODE_POLICIES': 'nodePolicies',
    'NODES': 'nodes',
    'DEFAULTS': 'defaults',
}
configuration_identifier_data = configuration_data()
foreach name, value : configuration_identifier_values
    configuration_identifier_data.set_quoted(name, value)
endforeach
configuration_identifiers = configure_file(
    input: 'identifiers.hpp.in',
    output: 'identifiers.hpp',
    configuration: configuration_identifier_data,
)

glaze_dep = dependency(
    'glaze',
    version: '=9.0.0',
    method: 'cmake',
    required: false,
)
if not glaze_dep.found()
    glaze_options = cmake.subproject_options()
    glaze_options.add_cmake_defines({'glaze_DEVELOPER_MODE': false})
    glaze_project = cmake.subproject('glaze', options: glaze_options)
    glaze_dep = glaze_project.dependency('glaze_glaze', include_type: 'system')
endif

configuration_schema = static_library(
    'ventricle-configuration-schema',
    [
        configuration_identifiers,
        'model/schema.cpp',
        'model/schema_context.cpp',
        'model/schema_values.cpp',
    ],
    install: true,
    override_options: ['cpp_std=c++23'],
    dependencies: [ventricle_core_dep, glaze_dep],
)
configuration_schema_dep = declare_dependency(
    link_with: configuration_schema,
    dependencies: ventricle_core_dep,
)

configuration = static_library(
    'ventricle-configuration',
    [
        configuration_identifiers,
        'model/parser/memory_identifiers_repository.cpp',
        'model/parser/nearest_scope_selection_policy.cpp',
        'model/parser/object_factory.cpp',
    ],
    install: true,
    dependencies: [ventricle_core_dep, glaze_dep],
)
configuration_dep = declare_dependency(
    link_with: configuration,
    dependencies: [ventricle_core_dep, glaze_dep],
)

subdir('tests')
