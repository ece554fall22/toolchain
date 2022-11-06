argparse_dep = dependency('argparse', required: true)
fmt_dep      = dependency('fmt', required: true)


sim_sources = files('main.cpp', 'cpu.cpp')
sim_exe = executable('sim', sim_sources,
                     dependencies: [argparse_dep, fmt_dep])
