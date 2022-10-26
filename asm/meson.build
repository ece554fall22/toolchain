argparse_dep = dependency('argparse', required: true)


asm_sources = files('main.cpp', 'lexer.cpp')
asm_exe = executable('asm', asm_sources,
                     dependencies: [argparse_dep])
