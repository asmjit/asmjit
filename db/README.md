AsmJit Instruction Database
---------------------------

This is a database of instructions that is used by AsmJit to generate its internal database and also assembler implementations. This project started initially as AsmDB, but was merged to AsmJit later to make the maintenance easier. The database was created in a way so that each instruction definition would only need a single line in JSON data. The data is then processed by architecture specific data readers that make the data canonical and ready for processing.

AsmJit database provides the following ISAs:

  * `isa_x86.json` - provides X86 instruction data (both 32-bit and 64-bit)
  * `isa_arm.json` - provides AArch32 instruction data (both ARM32 and THUMB)
  * `isa_a64.json` - provides AArch64 instruction data

To Be Documented
----------------

This project will be refactored and documented in the future.
