fmt_dep      = dependency('fmt', required: true)
doctest_dep  = dependency('doctest', required: true)

morph_inc = include_directories('.')
morph_src = files('decoder.cpp', 'encoder.cpp')
morph_deps = [fmt_dep]

morph_lib = static_library('libmorph',
    morph_src,
    include_directories: morph_inc,
    dependencies: [morph_deps])
libmorph_dep = declare_dependency(
    link_with: [morph_lib],
    include_directories: morph_inc,
    dependencies: [morph_deps])

test_instr_decoder = executable('test_instr_decoder',
    files('tests/instr_decoder.cpp'),
    dependencies: [doctest_dep, libmorph_dep])
test_instr_encoder = executable('test_instr_encoder',
    files('tests/instr_encoder.cpp'),
    dependencies: [doctest_dep, libmorph_dep])
test_varint = executable('test_varint',
    files('tests/varint.cpp'),
    dependencies: [doctest_dep, libmorph_dep])
test('instruction decoder', test_instr_decoder)
test('instruction encoder', test_instr_encoder)
test('varint utils', test_varint)
