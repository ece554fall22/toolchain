# brief notes about ontology: terms used in the codebase
- decoder: turn instructions into function calls to an `InstructionVisitor`
- encoder: turn function calls to the `Emitter` into an instruction stream
- emitter/`emit.h`: poorly named because it conflicts with the previous name but is different. effectively just a shim between the parsed AST and the encoder. switches on what mnemonic is used, and then pulls the appropriate operands out of the syntax tree and calls the encoder.
- `CPUInstructionProxy`: since the decoder just calls a visitor, this class acts as a visitor and proxies calls to `sim/instruction.h` instruction implementations.

