ventricle_core_dep = declare_dependency(
	compile_args: ['-DVENTRICLE_VERSION="' + meson.project_version() + '"'],
	dependencies: ns3_dep,
	include_directories: ventricle_include,
	sources: files(
		'defs.hpp',
		'type_glue.hpp',
		'version.hpp',
	),
)
