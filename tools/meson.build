# tools

tools_deps = []
if host_machine.system() == 'windows'
	tools_deps += ['../contrib/getopt-shim.c']
endif

tools_deps += [	'../contrib/pthread-shim.c']

srp_shared = []
tools_dependencies = []
if use_mbedtls
	srp_shared += 'srp_shared.c'
	tools_dependencies += mbedcrypto_lib
endif

use_libyaml = get_option('use_libyaml')
builtin_libyaml = get_option('builtin_libyaml')

if use_libyaml
	if not builtin_libyaml
		libyaml = dependency('libyaml',required: false)
		if not libyaml.found()
			libyaml = cc.find_library('libyaml', required: false)
			if not libyaml.found()
				builtin_libyaml = true
			endif
		endif
	endif
	if builtin_libyaml
		cmake = import('cmake')
		toolchainfile = ''
		if meson.is_cross_build()
			if meson.get_cross_property('mingw_toolchainfile')
				toolchainfile = '-DCMAKE_TOOLCHAIN_FILE='+meson.current_source_dir() + '/common/ci/x86_64-w64-mingw32.cmake'
			endif
		endif
		libyaml_proj = cmake.subproject('libyaml', cmake_options: ['-DENABLE_TESTING=OFF', '-DENABLE_PROGRAMS=OFF', '-DCMAKE_POSITION_INDEPENDENT_CODE=ON', toolchainfile], default_options: ['werror=false'])
		libyaml = libyaml_proj.dependency('yaml')
	endif
	tools_dependencies += [libyaml]
endif 

executable('ristsender',
	['ristsender.c', 'yamlparse.c', srp_shared, tools_deps, rev_target],
	link_with : [librist],
	dependencies: [
		threads,
		stdatomic_dependency,
		tools_dependencies
	],
	include_directories: inc,
	install: true)

executable('ristreceiver',
	['ristreceiver.c', 'yamlparse.c', srp_shared, tools_deps, rev_target],
	link_with : [librist],
	dependencies: [tools_dependencies],
	include_directories: inc,
	install: true)

executable('rist2rist',
        ['rist2rist.c', tools_deps],
        link_with : [librist],
		include_directories: inc,
        install: true)
if use_mbedtls
	executable('ristsrppasswd',
			['ristsrppasswd.c', '../contrib/srp.c', tools_deps],
			link_with : [librist],
			dependencies: [mbedcrypto_lib],
			include_directories: inc,
			install: true)
endif
