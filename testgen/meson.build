argparse_dep = dependency('argparse', required: true)
fmt_dep      = dependency('fmt', required: true)
pcg_dep      = dependency('pcg', required: true)


testgen_sources = files('main.cpp')
testgen_exe = executable('testgen', testgen_sources,
                     dependencies: [argparse_dep, fmt_dep, pcg_dep, libmorph_dep])
