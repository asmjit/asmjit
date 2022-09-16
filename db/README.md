AsmJit Instruction Database
---------------------------

This is a database of instructions that is used by AsmJit to generate its internal database and also assembler implementations. This project started initially as AsmDB, but was merged with AsmJit later to make the maintenance easier. The database was created in a way so that each instruction definition would only need a single line in JSON data. The data is then processed by architecture specific data readers that make the data canonical and ready for processing.

AsmJit database provides the following ISAs:

  * `x86data.js` - provides 32-bit and 64-bit X86 instructions
  * `armdata.js` - provides AArch32 instructions (both ARM32 and THUMB)
  * `a64data.json` - provides AArch64 instructions
  * `riscvdata.js` - provides RISC-V instructions

To Be Documented
----------------

This project will be refactored and documented in the future.
