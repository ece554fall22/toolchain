argparse_dep  = dependency('argparse', required: true)
fmt_dep       = dependency('fmt', required: true)
doctest_dep   = dependency('doctest', required: true)
json_dep      = dependency('nlohmann_json', version: '3.11.2', required: true)
linenoise_dep = dependency('linenoise', required: true)
eigen_dep    = dependency('eigen3', required: true)

sim_inc = include_directories('.')
sim_deps = [libmorph_dep, fmt_dep, eigen_dep, linenoise_dep]

libsim_sources = files('mem.cpp', 'trace.cpp', 'debugger.cpp')
libsim = static_library(
    'libsim', libsim_sources,
    include_directories: sim_inc,
    dependencies: sim_deps)

libsim_dep = declare_dependency(link_with: libsim, include_directories: sim_inc, dependencies: sim_deps)

sim_exe = executable('sim', files('main.cpp'),
                     dependencies: [libsim_dep, argparse_dep, json_dep, fmt_dep])

test_instruction_impl = executable('test_instruction_impl',
    'tests/instruction_impl.cpp',
    link_with: [libsim],
    dependencies: [doctest_dep] + sim_deps)
test('instruction implementations', test_instruction_impl)
