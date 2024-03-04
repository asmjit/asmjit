#include <asmjit/core.h>
#include <asmjit/asmjit.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include "asmjit_test_stress_x86_data.h"

using namespace asmjit;

template <typename T, typename V>
bool contains(const T& c, V v)
{
	const auto it = c.find(v);
	return it !=c.end();
}

#define ASSERT assert
#define DEBUG_JIT_PRINT_ASM 0

#define D(n)               ((0x0000)+(n)) //8
#define A(n)               ((0x0008)+(n)) //2
#define SWITCHKEY			 0x13A9
#define SP                   0x02EC
#define SP2                  0x02ED
#define GD(n)              ((0x02F5)+(n)) //1024, spare 352

#define BITS_SP                    10
#define MASK_SP                    ((1<<BITS_SP)-1)
#define MAX_SCRIPT_REGISTERS       (1<<BITS_SP)

#define RUNSCRIPT_OK			0
#define RUNSCRIPT_ERROR			1
#define RUNSCRIPT_SELFDELETE	2
#define RUNSCRIPT_STOPPED		3
#define RUNSCRIPT_SELFREMOVE	4

// Base flags, describing the relation
#define CMP_GT    0x01
#define CMP_LT    0x02
#define CMP_EQ    0x04
// Extra behavior flags
#define CMP_SETI  0x08 // In 'SETCMP', multiplies the output by 10000.
#define CMP_BOOL  0x10 // Does a "boolean comparison" (ex. 2 CMP_EQ 3 is true, since !2 == !3)

#define CMP_FLAGS 0x07
#define CMP_NE    (CMP_GT|CMP_LT)
#define CMP_GE    (CMP_GT|CMP_EQ)
#define CMP_LE    (CMP_LT|CMP_EQ)

typedef uint32_t pc_t;

struct ZasmFunction
{
	pc_t id;
	std::string name;
	pc_t start_pc;
	pc_t final_pc;
	bool may_yield;
	std::set<pc_t> called_by_functions;
	// Currently nothing needs this.
	// std::set<pc_t> calls_functions;
};

struct StructuredZasm
{
	std::vector<ZasmFunction> functions;
	std::set<pc_t> function_calls;
	std::map<pc_t, pc_t> start_pc_to_function;
	enum CallingMode {
		CALLING_MODE_UNKNOWN,
		CALLING_MODE_GOTO_GOTOR,
		CALLING_MODE_GOTO_RETURN,
		CALLING_MODE_CALLFUNC_RETURNFUNC,
	} calling_mode;
};

struct script_data {
	Instruction *zasm;
	size_t size;
};

int32_t run_script_int(bool)
{
	return RUNSCRIPT_OK;
};

int32_t get_register(int32_t){return 0;}
void set_register(int32_t, int32_t){}

StructuredZasm zasm_construct_structured(const script_data* script)
{
	// Find all function calls.
	std::set<pc_t> function_calls;
	std::set<pc_t> function_calls_goto_pc;
	std::map<pc_t, pc_t> function_calls_pc_to_pc;

	// Three forms of function calls over the ages:

	// 1) GOTO/GOTOR
	// The oldest looks like this:
	//
	//    SETV D2 (pc two after the GOTO)*10000
	//    PUSHR D2
	//    ... other ops to push the function args ...
	//    GOTO x
	// 
	// x: ...
	//    POP D3
	//    GOTOR D3
	//
	// GOTOR only ever used D3. POP D3 could instead be POPARGS.
	// Could also use D3 to set/push the return address.

	// 2) GOTO/RETURN
	//
	//    PUSHV (pc two after the GOTO)
	//    ... other ops to push the function args ...
	//    GOTO x
	// 
	// x: ...
	//    RETURN

	// 3) CALLFUNC/RETURNFUNC
	//
	//    CALLFUNC x
	// 
	// x: ...
	//    RETURNFUNC
	//
	// CALLFUNC pushes the return address onto a function call statck. RETURNFUNC pops from that stack.

	// There is nothing special marking the start or end of a function in ZASM. So,
	// the only way to contruct the bounds of each function is to search for function calls,
	// which gets the function starts. Then, the function ends are derived with these.
	// Note - if a function is not called at all, then the instructions for that function will
	// be part of an unreachable sequence of blocks in the prior function that was called.
	// Therefore, the instructions of uncalled functions should be pruned as part of zasm_optimize.

	// First determine if we have the simpler CALLFUNC instructions.
	auto calling_mode = StructuredZasm::CALLING_MODE_UNKNOWN;
	for (pc_t i = 0; i < script->size && !calling_mode; i++)
	{
		int command = script->zasm[i].command;
		switch (command)
		{
			case GOTOR:
				calling_mode = StructuredZasm::CALLING_MODE_GOTO_GOTOR;
				break;

			case RETURN:
				calling_mode = StructuredZasm::CALLING_MODE_GOTO_RETURN;
				break;

			case CALLFUNC:
			case RETURNFUNC:
				calling_mode = StructuredZasm::CALLING_MODE_CALLFUNC_RETURNFUNC;
				break;
		}
	}
	bool legacy_calling_mode =
		calling_mode == StructuredZasm::CALLING_MODE_GOTO_GOTOR || calling_mode == StructuredZasm::CALLING_MODE_GOTO_RETURN;

	// Starts with implicit first function ("run").
	std::set<pc_t> function_start_pcs_set = {0};

	for (pc_t i = 0; i < script->size; i++)
	{
		int command = script->zasm[i].command;

		bool is_function_call_like = false;
		if (command == CALLFUNC)
		{
			is_function_call_like = true;
		}
		else if (command == STARTDESTRUCTOR)
		{
			function_start_pcs_set.insert(i);
			continue;
		}
		else if (legacy_calling_mode && command == GOTO)
		{
			// Function calls are directly followed with a POP to restore the stack frame pointer.
			// PEEK is also possible via an optimization done by the compiler.
			int next_command = script->zasm[i + 1].command;
			is_function_call_like = (next_command == POP || next_command == PEEK) && script->zasm[i + 1].arg1 == D(4);
		}
		else
		{
			continue;
		}

		if (is_function_call_like && script->zasm[i].arg1 != -1)
		{
			function_calls.insert(i);
			function_start_pcs_set.insert(script->zasm[i].arg1);
			function_calls_pc_to_pc[i] = script->zasm[i].arg1;
			ASSERT(script->zasm[i].arg1 != i + 1);
		}
	}

	std::vector<pc_t> function_start_pcs(function_start_pcs_set.begin(), function_start_pcs_set.end());
	std::vector<pc_t> function_final_pcs;
	std::map<pc_t, pc_t> start_pc_to_function;
	{
		start_pc_to_function[0] = 0;

		pc_t next_fn_id = 1;
		for (int i = 1; i < function_start_pcs.size(); i++)
		{
			pc_t function_start_pc = function_start_pcs[i];
			function_final_pcs.push_back(function_start_pc - 1);
			start_pc_to_function[function_start_pc] = next_fn_id++;
		}
		// Don't include 0xFFFF as part of the last function.
		function_final_pcs.push_back(script->size - 2);

		// Just so std::lower_bound below will work for last function.
		function_start_pcs.push_back(script->size);
	}

	std::vector<ZasmFunction> functions;
	for (pc_t i = 0; i < function_final_pcs.size(); i++)
	{
		functions.push_back({i, "", function_start_pcs.at(i), function_final_pcs.at(i)});
	}

	for (auto [a, b] : function_calls_pc_to_pc)
	{
		auto it = std::lower_bound(function_start_pcs.begin(), function_start_pcs.end(), a);
		ASSERT(it != function_start_pcs.end());
		pc_t callee_pc = std::distance(function_start_pcs.begin(), it) - 1;

		it = std::lower_bound(function_start_pcs.begin(), function_start_pcs.end(), b);
		ASSERT(it != function_start_pcs.end());
		ASSERT(*it == b);
		pc_t call_pc = std::distance(function_start_pcs.begin(), it);

		functions[call_pc].called_by_functions.insert(callee_pc);
		// functions[callee_pc].calls_functions.insert(call_pc);
	}

	return {functions, function_calls, start_pc_to_function, calling_mode};
}

bool command_could_return_not_ok(int command)
{
	switch (command)
	{
	case 0xFFFF:
	case EWPNDEL:
	case GAMECONTINUE:
	case GAMEEND:
	case GAMERELOAD:
	case GAMESAVECONTINUE:
	case GAMESAVEQUIT:
	case ITEMDEL:
	case LWPNDEL:
	case NPCKICKBUCKET:
		return true;
	}
	return false;
}

bool command_is_wait(int command)
{
	switch (command)
	{
	case WAITFRAME:
	case WAITDRAW:
	case WAITTO:
	case WAITEVENT:
	case WAITFRAMESR:
		return true;
	}
	return false;
}

bool command_is_goto(int command)
{
	// GOTOR/return ops left out on purpose.
	switch (command)
	{
	case GOTO:
	case GOTOCMP:
	case GOTOLESS:
	case GOTOMORE:
	case GOTOTRUE:
	case GOTOFALSE:
		return true;
	}
	return false;
}

bool command_uses_comparison_result(int command)
{
	switch (command)
	{
	case GOTOTRUE:
	case GOTOFALSE:
	case GOTOMORE:
	case GOTOLESS:
	case GOTOCMP:
	case SETCMP:
	case SETTRUE:
	case SETTRUEI:
	case SETFALSE:
	case SETFALSEI:
	case SETMOREI:
	case SETLESSI:
	case SETMORE:
	case SETLESS:
	case STACKWRITEATVV_IF:
		return true;
	}
	return false;
}

struct refInfo {
	int pc, sp;
};

// struct JittedScriptHandle
// {
// 	JittedFunction fn;
// 	script_data *script;
// 	refInfo *ri;
// 	intptr_t call_stack_rets[100];
// 	uint32_t call_stack_ret_index;
// };

typedef int32_t (*JittedFunctionImpl)(int32_t *registers, int32_t *global_registers,
								  int32_t *stack, uint32_t *stack_index, uint32_t *pc,
								  intptr_t *call_stack_rets, uint32_t *call_stack_ret_index,
								  uint32_t *wait_index);
typedef JittedFunctionImpl JittedFunction ;

static JitRuntime rt;

struct CompilationState
{
	CallConvId calling_convention;
	// Some globals to prevent passing around everywhere
	size_t size;
	x86::Gp vRetVal;
	x86::Gp vSwitchKey;
	Label L_End;
	// Registers for the compiled function parameters.
	x86::Gp ptrRegisters;
	x86::Gp ptrGlobalRegisters;
	x86::Gp ptrStack;
	x86::Gp ptrStackIndex;
	x86::Gp ptrPc;
	x86::Gp ptrCallStackRets;
	x86::Gp ptrCallStackRetIndex;
	x86::Gp ptrWaitIndex;
};

class MyErrorHandler : public ErrorHandler
{
public:
	void handleError(Error err, const char *message, BaseEmitter *origin) override
	{
		// al_trace("AsmJit error: %s\n", message);
	}
};

static x86::Gp get_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r)
{
	x86::Gp val = cc.newInt32();
	if (r >= D(0) && r <= A(1))
	{
		cc.mov(val, x86::ptr_32(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.mov(val, x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.mov(val, vStackIndex);
		cc.imul(val, 10000);
	}
	else if (r == SP2)
	{
		cc.mov(val, vStackIndex);
	}
	else if (r == SWITCHKEY)
	{
		cc.mov(val, state.vSwitchKey);
	}
	else
	{
		// Call external get_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val);
	}
	return val;
}

static x86::Gp get_z_register_64(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r)
{
	x86::Gp val = cc.newInt64();
	if (r >= D(0) && r <= A(1))
	{
		cc.movsxd(val, x86::ptr_32(state.ptrRegisters, r * 4));
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.movsxd(val, x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4));
	}
	else if (r == SP)
	{
		cc.movsxd(val, vStackIndex);
		cc.imul(val, 10000);
	}
	else if (r == SP2)
	{
		cc.movsxd(val, vStackIndex);
	}
	else if (r == SWITCHKEY)
	{
		cc.movsxd(val, state.vSwitchKey);
	}
	else
	{
		// Call external get_register.
		x86::Gp val32 = cc.newInt32();
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, get_register, FuncSignatureT<int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setRet(0, val32);
		cc.movsxd(val, val32);
	}
	return val;
}

template <typename T>
static void set_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r, T val)
{
	if (r >= D(0) && r <= A(1))
	{
		cc.mov(x86::ptr_32(state.ptrRegisters, r * 4), val);
	}
	else if (r >= GD(0) && r <= GD(MAX_SCRIPT_REGISTERS))
	{
		cc.mov(x86::ptr_32(state.ptrGlobalRegisters, (r - GD(0)) * 4), val);
	}
	else if (r == SP || r == SP2)
	{
		// TODO
		// Z_error_fatal("Unimplemented: set SP");
	}
	else if (r == SWITCHKEY)
	{
		state.vSwitchKey = cc.newInt32();
		cc.mov(state.vSwitchKey, val);
	}
	else
	{
		// Call external set_register.
		InvokeNode *invokeNode;
		cc.invoke(&invokeNode, set_register, FuncSignatureT<void, int32_t, int32_t>(state.calling_convention));
		invokeNode->setArg(0, r);
		invokeNode->setArg(1, val);
	}
}

static void set_z_register(CompilationState& state, x86::Compiler &cc, x86::Gp vStackIndex, int r, x86::Mem mem)
{
	x86::Gp val = cc.newInt32();
	cc.mov(val, mem);
	set_z_register(state, cc, vStackIndex, r, val);
}

static void modify_sp(x86::Compiler &cc, x86::Gp vStackIndex, int delta)
{
	cc.add(vStackIndex, delta);
	cc.and_(vStackIndex, MASK_SP);
}

static void div_10000(x86::Compiler &cc, x86::Gp dividend)
{
	// Perform division by invariant multiplication.
	// https://clang.godbolt.org/z/c4qG3s9nW
	if (dividend.isType(RegType::kGp64))
	{
		x86::Gp input = cc.newInt64();
		cc.mov(input, dividend);

		x86::Gp r = cc.newInt64();
		cc.movabs(r, 3777893186295716171);
		cc.imul(r, r, dividend);
		cc.sar(r, 11);

		x86::Gp b = cc.newInt64();
		cc.mov(b, input);
		cc.sar(b, 63);
		cc.sub(r, b);

		cc.mov(dividend, r);
	}
	else if (dividend.isType(RegType::kGp32))
	{
		x86::Gp r = cc.newInt64();
		cc.movsxd(r, dividend);
		cc.sar(dividend, 31);
		cc.imul(r, r, 1759218605);
		cc.sar(r, 44);

		cc.sub(r.r32(), dividend);
		cc.mov(dividend, r.r32());
	}
	else
	{
		abort();
	}
}

static void zero(x86::Compiler &cc, x86::Gp reg)
{
	cc.xor_(reg, reg);
}

static void cast_bool(x86::Compiler &cc, x86::Gp reg)
{
	cc.test(reg, reg);
	cc.mov(reg, 0);
	cc.setne(reg.r8());
}

static void compile_compare(CompilationState& state, x86::Compiler &cc, std::map<int, Label> &goto_labels, x86::Gp vStackIndex, int command, int arg1, int arg2, int arg3)
{
	x86::Gp val = cc.newInt32();
	
	if(command == GOTOCMP)
	{
		auto lbl = goto_labels.at(arg1);
		switch(arg2 & CMP_FLAGS)
		{
			default:
				break;
			case CMP_GT:
				cc.jg(lbl);
				break;
			case CMP_GT|CMP_EQ:
				cc.jge(lbl);
				break;
			case CMP_LT:
				cc.jl(lbl);
				break;
			case CMP_LT|CMP_EQ:
				cc.jle(lbl);
				break;
			case CMP_EQ:
				cc.je(lbl);
				break;
			case CMP_GT|CMP_LT:
				cc.jne(lbl);
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				cc.jmp(lbl);
				break;
		}
	}
	else if (command == SETCMP)
	{
		cc.mov(val, 0);
		bool i10k = (arg2 & CMP_SETI);
		x86::Gp val2;
		if(i10k)
		{
			val2 = cc.newInt32();
			cc.mov(val2, 10000);
		}
		switch(arg2 & CMP_FLAGS)
		{
			default:
				break;
			case CMP_GT:
				if(i10k)
					cc.cmovg(val, val2);
				else cc.setg(val);
				break;
			case CMP_GT|CMP_EQ:
				if(i10k)
					cc.cmovge(val, val2);
				else cc.setge(val);
				break;
			case CMP_LT:
				if(i10k)
					cc.cmovl(val, val2);
				else cc.setl(val);
				break;
			case CMP_LT|CMP_EQ:
				if(i10k)
					cc.cmovle(val, val2);
				else cc.setle(val);
				break;
			case CMP_EQ:
				if(i10k)
					cc.cmove(val, val2);
				else cc.sete(val);
				break;
			case CMP_GT|CMP_LT:
				if(i10k)
					cc.cmovne(val, val2);
				else cc.setne(val);
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				if(i10k)
					cc.mov(val, 10000);
				else cc.mov(val, 1);
				break;
		}
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == GOTOTRUE)
	{
		cc.je(goto_labels.at(arg1));
	}
	else if (command == GOTOFALSE)
	{
		cc.jne(goto_labels.at(arg1));
	}
	else if (command == GOTOMORE)
	{
		cc.jge(goto_labels.at(arg1));
	}
	else if (command == GOTOLESS)
	{
		// if (get_qr(qr_GOTOLESSNOTEQUAL))
		// 	cc.jle(goto_labels.at(arg1));
		// else
			cc.jl(goto_labels.at(arg1));
	}
	else if (command == SETTRUE)
	{
		cc.mov(val, 0);
		cc.sete(val);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETTRUEI)
	{
		// https://stackoverflow.com/a/45183084/2788187
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmove(val, val2);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETFALSE)
	{
		cc.mov(val, 0);
		cc.setne(val);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETFALSEI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovne(val, val2);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETMOREI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovge(val, val2);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETLESSI)
	{
		cc.mov(val, 0);
		x86::Gp val2 = cc.newInt32();
		cc.mov(val2, 10000);
		cc.cmovle(val, val2);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETMORE)
	{
		cc.mov(val, 0);
		cc.setge(val);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if (command == SETLESS)
	{
		cc.mov(val, 0);
		cc.setle(val);
		set_z_register(state, cc, vStackIndex, arg1, val);
	}
	else if(command == STACKWRITEATVV_IF)
	{
		// Write directly value on the stack (arg1 to offset arg2)
		x86::Gp offset = cc.newInt32();
		cc.mov(offset, vStackIndex);
		if (arg2)
			cc.add(offset, arg2);
		auto cmp = arg3 & CMP_FLAGS;
		switch(cmp) //but only conditionally
		{
			case 0:
				break;
			case CMP_GT|CMP_LT|CMP_EQ:
				cc.mov(x86::ptr_32(state.ptrStack, offset, 2), arg1);
				break;
			default:
			{
				x86::Gp tmp = cc.newInt32();
				x86::Gp val = cc.newInt32();
				cc.mov(tmp, x86::ptr_32(state.ptrStack, offset, 2));
				cc.mov(val, arg1);
				switch(cmp)
				{
					case CMP_GT:
						cc.cmovg(tmp, val);
						break;
					case CMP_GT|CMP_EQ:
						cc.cmovge(tmp, val);
						break;
					case CMP_LT:
						cc.cmovl(tmp, val);
						break;
					case CMP_LT|CMP_EQ:
						cc.cmovle(tmp, val);
						break;
					case CMP_EQ:
						cc.cmove(tmp, val);
						break;
					case CMP_GT|CMP_LT:
						cc.cmovne(tmp, val);
						break;
					default:
						assert(false);
				}
				cc.mov(x86::ptr_32(state.ptrStack, offset, 2), tmp);
			}
		}
	}
	else
	{
		// Z_error_fatal("Unimplemented: %s", script_debug_command_to_string(command, arg1, arg2, arg3).c_str());
	}
}

// Defer to the ZASM command interpreter for 1+ commands.
static void compile_command_interpreter(CompilationState& state, x86::Compiler &cc, script_data *script, int i, int count, x86::Gp vStackIndex, bool is_wait = false)
{
	static int32_t jitted_uncompiled_command_count;

	x86::Gp reg = cc.newIntPtr();
	cc.mov(reg, (uint64_t)&jitted_uncompiled_command_count);
	cc.mov(x86::ptr_32(reg), count);

	cc.mov(x86::ptr_32(state.ptrPc), i);
	cc.mov(x86::ptr_32(state.ptrStackIndex), vStackIndex);

	InvokeNode *invokeNode;
	cc.invoke(&invokeNode, run_script_int, FuncSignatureT<int32_t, bool>(state.calling_convention));
	invokeNode->setArg(0, true);

	if (is_wait)
	{
		x86::Gp retVal = cc.newInt32();
		invokeNode->setRet(0, retVal);
		cc.cmp(retVal, RUNSCRIPT_OK);
		cc.jne(state.L_End);
		return;
	}

	bool could_return_not_ok = false;
	for (int j = 0; j < count; j++)
	{
		int index = i + j;
		if (index >= state.size)
			break;

		if (command_could_return_not_ok(script->zasm[index].command))
		{
			could_return_not_ok = true;
			break;
		}
	}

	if (could_return_not_ok)
	{
		invokeNode->setRet(0, state.vRetVal);
		cc.cmp(state.vRetVal, RUNSCRIPT_OK);
		cc.jne(state.L_End);
	}
}

static bool command_is_compiled(int command)
{
	if (command_is_wait(command))
		return true;

	if (command_uses_comparison_result(command))
		return true;

	switch (command)
	{
	// These commands are critical to control flow.
	case COMPARER:
	case COMPAREV:
	case COMPAREV2:
	case GOTO:
	case GOTOR:
	case QUIT:
	case RETURN:
	case CALLFUNC:
	case RETURNFUNC:

	// These commands modify the stack pointer, which is just a local copy. If these commands
	// were not compiled, then vStackIndex would have to be restored after compile_command_interpreter.
	case POP:
	case POPARGS:
	case PUSHR:
	case PUSHV:
	case PUSHARGSR:
	case PUSHARGSV:

	// These can be commented out to instead run interpreted. Useful for
	// singling out problematic instructions.
	case ABS:
	case ADDR:
	case ADDV:
	case ANDR:
	case ANDV:
	case CASTBOOLF:
	case CASTBOOLI:
	case CEILING:
	case DIVR:
	case DIVV:
	case FLOOR:
	case LOAD:
	case LOADD:
	case LOADI:
	case MAXR:
	case MAXV:
	case MINR:
	case MINV:
	case MODR:
	case MODV:
	case MULTR:
	case MULTV:
	case NOP:
	case PEEK:
	case SETR:
	case SETV:
	case STORE:
	case STORED:
	case STOREDV:
	case STOREI:
	case STOREV:
	case SUBR:
	case SUBV:
	case SUBV2:
	
	//
	case STACKWRITEATVV:
		return true;
	}

	return false;
}

// static void error(ScriptDebugHandle* debug_handle, script_data *script, std::string str)
// {
	// str = fmt::format("failed to compile type: {} index: {} name: {}\nerror: {}\n",
	// 				  ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str(), str);

	// al_trace("%s", str.c_str());
	// if (debug_handle)
	// 	debug_handle->print(CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK, str.c_str());

	// if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL)
	// {
	// 	abort();
	// }
// }

// Compile the entire ZASM script at once, into a single function.
JittedFunction jit_compile_script(script_data *script)
{
	if (script->size <= 1)
		return nullptr;

	CompilationState state;
	state.size = script->size;
	size_t size = state.size;

	// al_trace("[jit] compiling script type: %s index: %d size: %zu name: %s\n", ScriptTypeToString(script->id.type), script->id.index, size, script->meta.script_name.c_str());

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	CodeHolder code;
	JittedFunctionImpl fn;

	state.calling_convention = CallConvId::kHost;
	code.init(rt.environment());

	// MyErrorHandler myErrorHandler;
	// code.setErrorHandler(&myErrorHandler);

	StringLogger logger;
	if (DEBUG_JIT_PRINT_ASM)
	{
		code.setLogger(&logger);
	}

	x86::Compiler cc(&code);

	// Setup parameters.
	cc.addFunc(FuncSignatureT<int32_t, int32_t *, int32_t *, int32_t *, uint16_t *, uint32_t *, intptr_t *, uint32_t *, uint32_t *>(state.calling_convention));
	state.ptrRegisters = cc.newIntPtr("registers_ptr");
	state.ptrGlobalRegisters = cc.newIntPtr("global_registers_ptr");
	state.ptrStack = cc.newIntPtr("stack_ptr");
	state.ptrStackIndex = cc.newIntPtr("stack_index_ptr");
	state.ptrPc = cc.newIntPtr("pc_ptr");
	state.ptrCallStackRets = cc.newIntPtr("call_stack_rets_ptr");
	state.ptrCallStackRetIndex = cc.newIntPtr("call_stack_ret_index_ptr");
	state.ptrWaitIndex = cc.newIntPtr("wait_index_ptr");
	cc.setArg(0, state.ptrRegisters);
	cc.setArg(1, state.ptrGlobalRegisters);
	cc.setArg(2, state.ptrStack);
	cc.setArg(3, state.ptrStackIndex);
	cc.setArg(4, state.ptrPc);
	cc.setArg(5, state.ptrCallStackRets);
	cc.setArg(6, state.ptrCallStackRetIndex);
	cc.setArg(7, state.ptrWaitIndex);

	state.vRetVal = cc.newInt32("return_val");
	zero(cc, state.vRetVal); // RUNSCRIPT_OK

	x86::Gp vStackIndex = cc.newUInt32("stack_index");
	cc.mov(vStackIndex, x86::ptr_32(state.ptrStackIndex));

	x86::Gp vCallStackRetIndex = cc.newUInt32("call_stack_ret_index");
	cc.mov(vCallStackRetIndex, x86::ptr_32(state.ptrCallStackRetIndex));

	// Scripts yield on calls to WaitX, which means the function
	// this returns needs to take a parameter that represents which WaitX to jump to on re-entry.
	// Each WaitX instruction is a unique label. We use a jump table for this.
	// If jumpto is 0, that starts from the beginning of the entrypoint function.
	// If jumpto is >0, it uses the nth "WaitX" label.
	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("re-entry jump table");

	std::vector<Label> wait_frame_labels;
	Label L_Table = cc.newLabel();
	Label L_Start = cc.newLabel();
	state.L_End = cc.newLabel();
	JumpAnnotation *annotation = cc.newJumpAnnotation();
	annotation->addLabel(L_Start);
	for (size_t i = 0; i < size; i++)
	{
		int command = script->zasm[i].command;
		if (!command_is_wait(command))
			continue;

		Label label = cc.newLabel();
		wait_frame_labels.push_back(label);
		annotation->addLabel(label);
	}

	x86::Gp target = cc.newIntPtr("target");
	x86::Gp offset = cc.newIntPtr("offset");
	x86::Gp wait_index = cc.newUInt32("wait_index");
	cc.mov(wait_index, x86::ptr_32(state.ptrWaitIndex));
	cc.lea(offset, x86::ptr(L_Table));
	if (cc.is64Bit())
		cc.movsxd(target, x86::ptr_32(offset, wait_index.cloneAs(offset), 2));
	else
		cc.mov(target, x86::ptr_32(offset, wait_index.cloneAs(offset), 2));
	cc.add(target, offset);

	// Find all GOTOs.
	std::map<int, Label> goto_labels;
	for (size_t i = 0; i < size; i++)
	{
		int command = script->zasm[i].command;
		if (command != CALLFUNC && command != GOTO && command != GOTOTRUE
			&& command != GOTOFALSE && command != GOTOMORE && command != GOTOLESS
			&& command != GOTOCMP)
			continue;

		goto_labels[script->zasm[i].arg1] = cc.newLabel();
	}

	auto structured_zasm = zasm_construct_structured(script);

	// Create a return label for every function call.
	std::map<int, Label> call_pc_to_return_label;
	for (pc_t pc : structured_zasm.function_calls)
	{
		call_pc_to_return_label[pc] = cc.newLabel();
	}

	// Create a jump annotation for the start of every function.
	std::vector<JumpAnnotation *> function_jump_annotations;
	for (int i = 0; i < structured_zasm.functions.size(); i++)
	{
		function_jump_annotations.push_back(cc.newJumpAnnotation());
	}

	// Map all RETURN to the enclosing function.
	std::map<int, int> return_to_function_id;
	{
		int cur_function_id = 0;
		for (size_t i = 0; i < size; i++)
		{
			if (structured_zasm.functions.size() > cur_function_id + 1 && structured_zasm.functions.at(cur_function_id + 1).start_pc == i)
				cur_function_id += 1;

			int command = script->zasm[i].command;
			if (command == RETURNFUNC || command == RETURN || command == GOTOR)
			{
				return_to_function_id[i] = cur_function_id;
			}
		}
	}

	// Annotate all function RETURNs to their calls, to help asmjit with liveness analysis.
	for (int function_call_pc : structured_zasm.function_calls)
	{
		int goto_pc = script->zasm[function_call_pc].arg1;
		auto it = structured_zasm.start_pc_to_function.find(goto_pc);
		int function_index = it->second;
		function_jump_annotations[function_index]->addLabel(call_pc_to_return_label.at(function_call_pc));
	}

	cc.jmp(target, annotation);
	cc.bind(L_Start);

	// Next, transform each ZASM command to the equivalent assembly.
	size_t label_index = 0;

	// cc.setInlineComment does not make a copy of the string, so we need to keep
	// comment strings around a bit longer than the invocation.
	std::string comment;

	for (size_t i = 0; i < size; i++)
	{
		auto& op = script->zasm[i];
		auto arg1 = op.arg1;
		auto arg2 = op.arg2;
		auto arg3 = op.arg3;
		// auto argvec = op.vecptr;
		// auto argstr = op.strptr;
		int command = op.command;

		if (contains(goto_labels, i))
		{
			cc.bind(goto_labels.at(i));
		}

		if (command_uses_comparison_result(command))
		{
			compile_compare(state, cc, goto_labels, vStackIndex, command, op.arg1, op.arg2, op.arg3);
			continue;
		}

		if (command_is_wait(command))
		{
			// Wait commands normally yield back to the engine, however there are some
			// special cases where it does not. For example, when WAITFRAMESR arg is 0.
			cc.mov(x86::ptr_32(state.ptrWaitIndex), label_index + 1);
			// This will jump to L_End, but only if actually waiting.
			compile_command_interpreter(state, cc, script, i, 1, vStackIndex, true);
			cc.bind(wait_frame_labels[label_index]);
			label_index += 1;
			continue;
		}

		if (!command_is_compiled(command))
		{
			// if (DEBUG_JIT_PRINT_ASM)
			// {
			// 	std::string command_str =
			// 		script_debug_command_to_string(command, op.arg1, op.arg2, op.arg3, op.vecptr, op.strptr);
			// 	cc.setInlineComment((comment = fmt::format("{} {}", i, command_str)).c_str());
			// 	cc.nop();
			// }

			// Every command that is not compiled to assembly must go through the regular interpreter function.
			// In order to reduce function call overhead, we call into the interpreter function in batches.
			int uncompiled_command_count = 1;
			for (int j = i + 1; j < size; j++)
			{
				if (command_is_compiled(script->zasm[j].command))
					break;
				if (contains(goto_labels, j))
					break;
				if (contains(structured_zasm.start_pc_to_function, j))
					break;

				uncompiled_command_count += 1;
				// if (DEBUG_JIT_PRINT_ASM)
				// {
				// 	auto& op = script->zasm[j];
				// 	std::string command_str =
				// 		script_debug_command_to_string(op.command, op.arg1, op.arg2, op.arg3, op.vecptr, op.strptr);
				// 	cc.setInlineComment((comment = fmt::format("{} {}", j, command_str)).c_str());
				// 	cc.nop();
				// }
			}

			compile_command_interpreter(state, cc, script, i, uncompiled_command_count, vStackIndex);
			i += uncompiled_command_count - 1;
			continue;
		}

		// Every command here must be reflected in command_is_compiled!
		switch (command)
		{
		case NOP:
			if (DEBUG_JIT_PRINT_ASM)
				cc.nop();
			break;
		case QUIT:
		{
			compile_command_interpreter(state, cc, script, i, 1, vStackIndex);
			cc.mov(state.vRetVal, RUNSCRIPT_STOPPED);
			cc.mov(x86::ptr_32(state.ptrWaitIndex), 0);
			cc.jmp(state.L_End);
		}
		break;
		case CALLFUNC:
			//Normally pushes a return address to the 'ret_stack'
			//...but we can ignore that when jitted
		[[fallthrough]];
		case GOTO:
		{
			if (contains(structured_zasm.function_calls, i))
			{
				// https://github.com/asmjit/asmjit/issues/286
				x86::Gp address = cc.newIntPtr();
				cc.lea(address, x86::qword_ptr(call_pc_to_return_label.at(i)));
				cc.mov(x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3), address);
				cc.add(vCallStackRetIndex, 1);
				cc.jmp(goto_labels.at(arg1));
				cc.bind(call_pc_to_return_label.at(i));
			}
			else
			{
				cc.jmp(goto_labels.at(arg1));
			}
		}
		break;

		// GOTOR is pretty much RETURN - was only used to return to the call location in scripts
		// compiled before RETURN existed.
		// Note: for GOTOR the return pc is in a register, but we just ignore it and instead use
		// the function call return label.
		case GOTOR:
		case RETURN:
		{
			// Note: for RETURN the return pc is on the stack, but we just ignore it and instead use
			// the function call return label.
			if (command == RETURN)
				modify_sp(cc, vStackIndex, 1);

			cc.sub(vCallStackRetIndex, 1);
			x86::Gp address = cc.newIntPtr();
			cc.mov(address, x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3));

			int function_index = return_to_function_id.at(i);
			// if (function_jump_annotations.size() <= function_index)
			// {
			// 	error(debug_handle, script, fmt::format("failed to resolve function return! i: {} function_index: {}", i, function_index));
			// 	return nullptr;
			// }
			cc.jmp(address, function_jump_annotations[function_index]);
		}
		break;
		case RETURNFUNC:
		{
			//Normally the return address is on the 'ret_stack'
			//...but we can ignore that when jitted

			cc.sub(vCallStackRetIndex, 1);
			x86::Gp address = cc.newIntPtr();
			cc.mov(address, x86::qword_ptr(state.ptrCallStackRets, vCallStackRetIndex, 3));

			int function_index = return_to_function_id.at(i);
			if (function_jump_annotations.size() <= function_index)
			{
				// error(debug_handle, script, fmt::format("failed to resolve function return! i: {} function_index: {}", i, function_index));
				return nullptr;
			}
			cc.jmp(address, function_jump_annotations[function_index]);
		}
		break;
		case STACKWRITEATVV:
		{
			// Write directly value on the stack (arg1 to offset arg2)
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, vStackIndex);
			if (arg2)
				cc.add(offset, arg2);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), arg1);
		}
		break;
		case PUSHV:
		{
			modify_sp(cc, vStackIndex, -1);
			cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), arg1);
		}
		break;
		case PUSHR:
		{
			// Grab value from register and push onto stack.
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			modify_sp(cc, vStackIndex, -1);
			cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), val);
		}
		break;
		case PUSHARGSR:
		{
			if(arg2 < 1) break; //do nothing
			// Grab value from register and push onto stack, repeatedly
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			for(int q = 0; q < arg2; ++q)
			{
				modify_sp(cc, vStackIndex, -1);
				cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), val);
			}
		}
		break;
		case PUSHARGSV:
		{
			if(arg2 < 1) break; //do nothing
			// Push value onto stack, repeatedly
			for(int q = 0; q < arg2; ++q)
			{
				modify_sp(cc, vStackIndex, -1);
				cc.mov(x86::ptr_32(state.ptrStack, vStackIndex, 2), arg1);
			}
		}
		break;
		case SETV:
		{
			// Set register to immediate value.
			set_z_register(state, cc, vStackIndex, arg1, arg2);
		}
		break;
		case SETR:
		{
			// Set register arg1 to value of register arg2.
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case LOAD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			if (arg2)
				cc.add(offset, arg2);

			set_z_register(state, cc, vStackIndex, arg1, x86::ptr_32(state.ptrStack, offset, 2));
		}
		break;
		case LOADD:
		{
			// Set register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			if (arg2)
				cc.add(offset, arg2);
			div_10000(cc, offset);

			set_z_register(state, cc, vStackIndex, arg1, x86::ptr_32(state.ptrStack, offset, 2));
		}
		break;
		case LOADI:
		{
			// Set register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, vStackIndex, arg2);
			div_10000(cc, offset);

			set_z_register(state, cc, vStackIndex, arg1, x86::ptr_32(state.ptrStack, offset, 2));
		}
		break;
		case STORE:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, arg2);
			cc.add(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));

			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), val);
		}
		break;
		case STOREV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			if (arg2)
				cc.add(offset, arg2);
			
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), arg1);
		}
		break;
		case STORED:
		{
			// Write from register to a value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, arg2);
			cc.add(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			div_10000(cc, offset);
			
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), val);
		}
		break;
		case STOREDV:
		{
			// Write directly value on the stack (offset is arg2 + rSFRAME register).
			x86::Gp offset = cc.newInt32();
			cc.mov(offset, x86::ptr_32(state.ptrRegisters, rSFRAME * 4));
			if (arg2)
				cc.add(offset, arg2);
			div_10000(cc, offset);
			
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), arg1);
		}
		break;
		case STOREI:
		{
			// Write from register to a value on the stack (offset is register at arg2).
			x86::Gp offset = get_z_register(state, cc, vStackIndex, arg2);
			div_10000(cc, offset);

			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.mov(x86::ptr_32(state.ptrStack, offset, 2), val);
		}
		break;
		case POP:
		{
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStack, vStackIndex, 2));
			modify_sp(cc, vStackIndex, 1);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case POPARGS:
		{
			// int32_t num = sarg2;
			// ri->sp += num;
			modify_sp(cc, vStackIndex, arg2);

			// word read = (ri->sp-1) & MASK_SP;
			x86::Gp read = cc.newInt32();
			cc.mov(read, vStackIndex);
			cc.sub(read, 1);
			cc.and_(read, MASK_SP);

			// int32_t value = SH::read_stack(read);
			// set_register(sarg1, value);
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStack, read, 2));
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ABS:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp y = cc.newInt32();
			cc.mov(y, val);
			cc.sar(y, 31);
			cc.xor_(val, y);
			cc.sub(val, y);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CASTBOOLI:
		{
			// https://clang.godbolt.org/z/W8PM4j33b
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.neg(val);
			cc.sbb(val, val);
			cc.and_(val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CASTBOOLF:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cast_bool(cc, val);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ADDV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.add(val, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ADDR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.add(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ANDV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);

			div_10000(cc, val);
			cc.and_(val, arg2 / 10000);
			cc.imul(val, 10000);

			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case ANDR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);

			div_10000(cc, val);
			div_10000(cc, val2);
			cc.and_(val, val2);
			cc.imul(val, 10000);

			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MAXR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MAXV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val2, val);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MINR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MINV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = cc.newInt32();
			cc.mov(val2, arg2);
			cc.cmp(val, val2);
			cc.cmovge(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case MODV:
		{
			if (arg2 == 0)
			{
				x86::Gp val = cc.newInt32();
				zero(cc, val);
				set_z_register(state, cc, vStackIndex, arg1, val);
				continue;
			}

			// https://stackoverflow.com/a/8022107/2788187
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			if (arg2 > 0 && (arg2 & (-arg2)) == arg2)
			{
				// Power of 2.
				// Because numbers in zscript are fixed point, "2" is really "20000"... so this won't
				// ever really be utilized.
				cc.and_(val, arg2 - 1);
				set_z_register(state, cc, vStackIndex, arg1, val);
			}
			else
			{
				x86::Gp divisor = cc.newInt32();
				cc.mov(divisor, arg2);
				x86::Gp rem = cc.newInt32();
				zero(cc, rem);
				cc.cdq(rem, val);
				cc.idiv(rem, val, divisor);
				set_z_register(state, cc, vStackIndex, arg1, rem);
			}
		}
		break;
		case MODR:
		{
			x86::Gp dividend = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp divisor = get_z_register(state, cc, vStackIndex, arg2);

			Label do_set_register = cc.newLabel();

			x86::Gp rem = cc.newInt32();
			zero(cc, rem);

			// Prevent division by zero. Result will be zero.
			cc.test(divisor, divisor);
			cc.jz(do_set_register);

			cc.cdq(rem, dividend);
			cc.idiv(rem, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, vStackIndex, arg1, rem);
		}
		break;
		case SUBV:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			cc.sub(val, arg2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case SUBR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);
			cc.sub(val, val2);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case SUBV2:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			x86::Gp result = cc.newInt32();
			cc.mov(result, arg1);
			cc.sub(result, val);
			set_z_register(state, cc, vStackIndex, arg2, result);
		}
		break;
		case MULTV:
		{
			x86::Gp val = get_z_register_64(state, cc, vStackIndex, arg1);
			cc.imul(val, arg2);
			div_10000(cc, val);
			set_z_register(state, cc, vStackIndex, arg1, val.r32());
		}
		break;
		case MULTR:
		{
			x86::Gp val = get_z_register_64(state, cc, vStackIndex, arg1);
			x86::Gp val2 = get_z_register_64(state, cc, vStackIndex, arg2);
			cc.imul(val, val2);
			div_10000(cc, val);
			set_z_register(state, cc, vStackIndex, arg1, val.r32());
		}
		break;
		// TODO guard for div by zero
		case DIVV:
		{
			x86::Gp dividend = get_z_register_64(state, cc, vStackIndex, arg1);
			int val2 = arg2;

			cc.imul(dividend, 10000);
			x86::Gp divisor = cc.newInt64();
			cc.mov(divisor, val2);
			x86::Gp dummy = cc.newInt64();
			zero(cc, dummy);
			cc.cqo(dummy, dividend);
			cc.idiv(dummy, dividend, divisor);

			set_z_register(state, cc, vStackIndex, arg1, dividend.r32());
		}
		break;
		case DIVR:
		{
			x86::Gp dividend = get_z_register_64(state, cc, vStackIndex, arg1);
			x86::Gp divisor = get_z_register_64(state, cc, vStackIndex, arg2);

			Label do_division = cc.newLabel();
			Label do_set_register = cc.newLabel();

			// If zero, result is (sign(dividend) * MAX_SIGNED_32)
			// TODO how expensive is this check? Maybe we can add a new QR that makes div-by-zero an error
			// and omit these safeguards.
			cc.test(divisor, divisor);
			cc.jnz(do_division);
			x86::Gp sign = cc.newInt64();
			cc.mov(sign, dividend);
			cc.sar(sign, 63);
			cc.or_(sign, 1);
			cc.mov(dividend.r32(), sign.r32());
			cc.imul(dividend.r32(), (2147483647));
			cc.jmp(do_set_register);

			// Else do the actual division.
			cc.bind(do_division);
			cc.imul(dividend, 10000);
			x86::Gp dummy = cc.newInt64();
			zero(cc, dummy);
			cc.cqo(dummy, dividend);
			cc.idiv(dummy, dividend, divisor);

			cc.bind(do_set_register);
			set_z_register(state, cc, vStackIndex, arg1, dividend.r32());
		}
		break;
		case COMPAREV:
		{
			int val = arg2;
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg1);

			if (script->zasm[i + 1].command == GOTOCMP || script->zasm[i + 1].command == SETCMP)
			{
				if (script->zasm[i + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		case COMPAREV2:
		{
			int val = arg1;
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg2);

			if (script->zasm[i + 1].command == GOTOCMP || script->zasm[i + 1].command == SETCMP)
			{
				if (script->zasm[i + 1].arg2 & CMP_BOOL)
				{
					val = val ? 1 : 0;
					cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		case COMPARER:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg2);
			x86::Gp val2 = get_z_register(state, cc, vStackIndex, arg1);

			if (script->zasm[i + 1].command == GOTOCMP || script->zasm[i + 1].command == SETCMP)
			{
				if (script->zasm[i + 1].arg2 & CMP_BOOL)
				{
					cast_bool(cc, val);
					cast_bool(cc, val2);
				}
			}

			cc.cmp(val2, val);
		}
		break;
		// https://gcc.godbolt.org/z/r9zq67bK1
		case FLOOR:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 9);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case CEILING:
		{
			x86::Gp val = get_z_register(state, cc, vStackIndex, arg1);
			x86::Xmm y = cc.newXmm();
			x86::Mem mem = cc.newQWordConst(ConstPoolScope::kGlobal, 4547007122018943789);
			cc.cvtsi2sd(y, val);
			cc.mulsd(y, mem);
			cc.roundsd(y, y, 10);
			cc.cvttsd2si(val, y);
			cc.imul(val, val, 10000);
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		case PEEK:
		{
			x86::Gp val = cc.newInt32();
			cc.mov(val, x86::ptr_32(state.ptrStack, vStackIndex, 2));
			set_z_register(state, cc, vStackIndex, arg1, val);
		}
		break;
		default:
		{
			// error(debug_handle, script, fmt::format("unhandled command: {}", command));
			return nullptr;
		}
		}
	}

	if (DEBUG_JIT_PRINT_ASM)
	{
		cc.setInlineComment("end commands");
		cc.nop();
	}

	cc.bind(state.L_End);

	// Persist stack pointer.
	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("persist stack pointer");
	cc.mov(x86::ptr_32(state.ptrStackIndex), vStackIndex);

	if (DEBUG_JIT_PRINT_ASM)
		cc.setInlineComment("persist call stack ret pointer");
	cc.mov(x86::ptr_32(state.ptrCallStackRetIndex), vCallStackRetIndex);

	cc.ret(state.vRetVal);
	cc.endFunc();

	// Relative int32_t offsets of `L_XXX - L_Table`.
	cc.bind(L_Table);
	cc.embedLabelDelta(L_Start, L_Table, 4);
	for (auto label : wait_frame_labels)
	{
		cc.embedLabelDelta(label, L_Table, 4);
	}

	for (auto &it : call_pc_to_return_label)
	{
		cc.embedLabelDelta(it.second, L_Table, 4);
	}

	end_time = std::chrono::steady_clock::now();
	int32_t preprocess_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	start_time = end_time;

	cc.finalize();
	rt.add(&fn, &code);

	end_time = std::chrono::steady_clock::now();
	int32_t compile_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	// al_trace("[jit] finished script %s %d. time: %d ms\n", ScriptTypeToString(script->id.type), script->id.index, preprocess_ms + compile_ms);

	if (fn)
	{
		printf("success\n");
	}
	else
	{
		// error(debug_handle, script, "failed to compile");
		printf("failure\n");
	}

	return fn;
}

bool stress_compiler()
{
	script_data script = {code, sizeof(code)/sizeof(Instruction)};
	return (bool)jit_compile_script(&script);
}
