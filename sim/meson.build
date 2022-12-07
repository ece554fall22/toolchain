argparse_dep  = dependency('argparse', required: true)
fmt_dep       = dependency('fmt', required: true)
doctest_dep   = dependency('doctest', required: true)
json_dep      = dependency('nlohmann_json', version: '3.11.2', required: true)
linenoise_dep = dependency('linenoise', required: true)

sim_inc = include_directories('.')
sim_deps = [libmorph_dep, fmt_dep, linenoise_dep]

libsim_sources = files('mem.cpp', 'trace.cpp', 'debugger.cpp')
libsim = static_library(
    'libsim', libsim_sources,
    include_directories: sim_inc,
    dependencies: sim_deps)

sim_exe = executable('sim', files('main.cpp'),
                     include_directories: sim_inc,
                     link_with: [libsim],
                     dependencies: [argparse_dep, json_dep] + sim_deps)

test_instruction_impl = executable('test_instruction_impl',
    'tests/instruction_impl.cpp',
    link_with: [libsim],
    dependencies: [doctest_dep] + sim_deps)
test('instruction implementations', test_instruction_impl)
