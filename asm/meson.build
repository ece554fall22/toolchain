argparse_dep = dependency('argparse', required: true)
fmt_dep      = dependency('fmt', required: true)


asm_sources = files('main.cpp', 'lexer.cpp', 'parser.cpp')
asm_exe = executable('asm', asm_sources,
                     dependencies: [argparse_dep, fmt_dep, libmorph_dep])
