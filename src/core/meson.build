ventricle_core_dep = declare_dependency(
	compile_args: ['-DVENTRICLE_VERSION="' + meson.project_version() + '"'],
	dependencies: [abseil_dep, ns3_dep],
	include_directories: ventricle_include,
	sources: files(
		'defs.hpp',
		'simulation/nodes.hpp',
		'simulation/listeners.hpp',
		'type_glue.hpp',
		'version.hpp',
	),
)

subdir('tests')
