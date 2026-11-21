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
        'model/parser/memory_identifiers_repository.cpp',
        'model/parser/nearest_scope_selection_policy.cpp',
    ],
    install: true,
    dependencies: ventricle_core_dep,
)
configuration_dep = declare_dependency(
    link_with: configuration,
    dependencies: ventricle_core_dep,
)

subdir('tests')
