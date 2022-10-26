from isa import ISA

with open('isa.yml') as f: isa = ISA.load(f)

def gen_instr_page(instr):
	print('\\newpage')
	print(f"\\subsection{{{instr['mnemonic']}}}")

for instr in isa.instructions:
	gen_instr_page(instr)