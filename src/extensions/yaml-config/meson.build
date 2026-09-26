glaze_dep = dependency('glaze', version: '=9.0.0', method: 'cmake', required: false)
if not glaze_dep.found()
  glaze_options = cmake.subproject_options()
  glaze_options.add_cmake_defines({'glaze_DEVELOPER_MODE': false})
  glaze_project = cmake.subproject('glaze', options: glaze_options)
  glaze_dep = glaze_project.dependency('glaze_glaze', include_type: 'system')
endif

yaml_config_library = static_library(
  'ventricle-yaml-config',
  'model/schema.cpp',
  override_options: ['cpp_std=c++23'],
  dependencies: [ventricle_core_dep, glaze_dep],
)
yaml_config_dep = declare_dependency(
  link_with: yaml_config_library,
  dependencies: ventricle_core_dep,
)
