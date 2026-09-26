dl_dep = meson.get_compiler('cpp').find_library('dl', required: host_machine.system() == 'linux')
dynamic_loader = static_library(
  'ventricle-dynamic-loader',
  'model/loader.cpp',
  dependencies: [ventricle_core_dep, dl_dep],
)
dynamic_loader_dep = declare_dependency(
  link_with: dynamic_loader,
  dependencies: [ventricle_core_dep, dl_dep],
)

if python.found()
  subdir('tests')
endif
