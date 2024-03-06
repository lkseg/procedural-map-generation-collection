# https://mesonbuild.com/Build-targets.html
project('cgui', 'cpp', 'c', default_options: ['debug=false', 'optimization=2', 'cpp_std=c++20'])

add_global_arguments(['-Wno-reorder', '-Wfatal-errors', '-Werror=return-type', '-Wno-unused-result', '-Wno-narrowing', '-Wno-sign-compare'], language : 'cpp')

debug_checks = get_option('debug_checks')
if debug_checks
    add_project_arguments(['-DXDEBUG'], language: 'cpp')
endif

# add_global_arguments(['-Wno-enum-compare'], language : 'cpp')


comp = meson.get_compiler('cpp')
dir = meson.current_source_dir()

src = []
incl = []
lib = []
incl += include_directories('include/imgui')
incl += include_directories('include') # imgui wants GLFW/
incl += include_directories('include/stb')
incl += include_directories('include/FastNoiseLite')



incl += include_directories('src')
incl += include_directories('src/util')
incl += include_directories('src/voronoi')


# incl += include_directories('include/GLFW')

subdir('src')
subdir('src/util')
subdir('src/voronoi')

subdir('include/imgui')

if host_machine.system()  == 'linux'
    rl_dir = dir + '/lib/linux/'
    lib += comp.find_library('glfw3', dirs: rl_dir)
elif host_machine.system()  == 'windows'
    message('windows @todo')
endif


enable_tests = get_option('compile_tests')
if not enable_tests
    executable('prog', src,  link_with: [], dependencies: lib, include_directories: incl, link_args: ['-lGL'])
else
    fs = import('fs') # module

    test_src = []
    foreach it: src
        if fs.name(it) not in ['main.cpp']
            test_src += it
        endif
    endforeach
    src = test_src
    
    src += files('src/tests.cpp')
    tests = executable('prog', src,  link_with: [], dependencies: lib, include_directories: incl, link_args: ['-lGL'])
    test('default test', tests)  
endif



# link_args: '-lm' # math stuff
# libc = dependency('libc', version : '>=1.2.8')

