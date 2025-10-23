#include "object_code.h"
#include "utils.h"

#define MAX_VARS_PER_FUNCTION 200

static VarLocation var_map[MAX_VARS_PER_FUNCTION];
static int var_count = 0;
static int current_stack_offset = 0;

// Argument registers for x86-64 calling convention
const char* arg_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

/* Get the stack offset for a variable
 * If the variable is not yet mapped, assign a new offset
 * Stack grows downwards, so offsets are negative
 */
static int get_var_offset(const char* name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(var_map[i].name, name) == 0) {
            return var_map[i].offset;
        }
    }
    current_stack_offset -= 8; // Allocate 8 bytes for the new variable
    var_map[var_count].name = my_strdup(name);
    var_map[var_count].offset = current_stack_offset; // Set offset for variable
    var_count++;
    return current_stack_offset;
}

/* Get the operand string for a given variable
 * Handles variables, constants, and labels
 * Formats the operand appropriately for assembly output
 * Variables are accessed via their stack offset
 * Constants are prefixed with '$'
 * Labels are used directly
 */
static void get_operand_str(INFO* var, char* buf, size_t buf_size) {
    if (var == NULL || var->id.name == NULL) {
        buf[0] = '\0';
        return;
    }
    char* name = var->id.name;

    // Check if it's a label, constant, or variable
    if (name[0] == 'L') {
        snprintf(buf, buf_size, "%s", name);
    } else if (isdigit(name[0]) || (name[0] == '-' && isdigit(name[1]))) {
        snprintf(buf, buf_size, "$%s", name);
    } else {
        snprintf(buf, buf_size, "%d(%%rbp)", get_var_offset(name));
    }
}

/* Main function to generate x86-64 assembly code from intermediate code
 * Outputs the assembly code to the provided file pointer
 * Handles function prologues/epilogues, arithmetic operations, control flow, and function calls
 * Uses a simple mapping of temporaries to stack offsets
 */
void generate_object_code(FILE* out_file) {
    Instr* code = get_intermediate_code();
    int code_size = get_code_size();
    int param_count = 0;
    int stack_params = 0; // Count parameters that need to go on stack

    fprintf(out_file, ".extern printf\n");
    fprintf(out_file, ".section .rodata\n");
    fprintf(out_file, "fmt: .string \"%%ld\\n\"\n");
    fprintf(out_file, ".text\n");

    for (int i = 0; i < code_size; ++i) {
        Instr* instr = &code[i]; // Get instruction from intermediate code structure
        char op1[64], op2[64], dest[64]; // Buffers for operand strings

        switch (instr->instruct->instruct.type_instruct) {
            case I_EXTERN:
                fprintf(out_file, ".globl %s\n", instr->var1->id.name);
                break;

            case I_ENTER: {
                const char* func_name = instr->var1->id.name;

                AST_NODE* func_node = NULL;
                // Search for the function declaration node in the AST
                for (AST_ROOT* cur = head_ast; cur != NULL; cur = cur->next) {
                    if (cur->sentence->info->type == AST_METHOD_DECL &&
                        strcmp(cur->sentence->info->method_decl.name, func_name) == 0) {
                        func_node = cur->sentence;
                        break;
                    }
                }

                var_count = 0;
                current_stack_offset = 0;
                int end_func_idx = i;
                // Search for the I_LEAVE instr for this function
                for (int j = i + 1; j < code_size; ++j) {
                    if (code[j].instruct->instruct.type_instruct == I_LEAVE &&
                        strcmp(code[j].var1->id.name, func_name) == 0) {
                        end_func_idx = j;
                        break;
                    }
                }
                for (int k = i; k <= end_func_idx; ++k) {
                    // Get stack offsets for all variables used in the function
                    if (code[k].var1 && code[k].var1->id.name && !isdigit(code[k].var1->id.name[0]) && code[k].var1->id.name[0] != 'L') get_var_offset(code[k].var1->id.name);
                    if (code[k].var2 && code[k].var2->id.name && !isdigit(code[k].var2->id.name[0]) && code[k].var2->id.name[0] != 'L') get_var_offset(code[k].var2->id.name);
                    if (code[k].reg  && code[k].reg->id.name  && !isdigit(code[k].reg->id.name[0])  && code[k].reg->id.name[0] != 'L') get_var_offset(code[k].reg->id.name);
                }
                int total_stack_size = -current_stack_offset;
                // Align stack to 16 bytes
                if (total_stack_size % 16 != 0) {
                    total_stack_size += 16 - (total_stack_size % 16);
                }

                var_count = 0;
                current_stack_offset = 0;

                // Function call prologue
                fprintf(out_file, "\n.globl %s\n", func_name);
                fprintf(out_file, "%s:\n", func_name);
                fprintf(out_file, "  pushq %%rbp\n");
                fprintf(out_file, "  movq %%rsp, %%rbp\n");
                if (total_stack_size > 0) {
                    fprintf(out_file, "  subq $%d, %%rsp\n", total_stack_size);
                }

                // Move arguments to the stack if they exist
                if (func_node) {
                    ARGS_LIST* arg_list = func_node->info->method_decl.args;
                    int arg_idx = 0;
                    while (arg_list) {
                        const char* arg_name = arg_list->arg->name;
                        int offset = get_var_offset(arg_name);
                        if (arg_idx < 6) {
                            // First 6 arguments come from registers
                            fprintf(out_file, "  movq %s, %d(%%rbp)\n", arg_regs[arg_idx], offset);
                        } else {
                            // Arguments 7+ are already on the stack (pushed by caller)
                            // They are at positive offsets from %rbp:
                            // rbp+16 is the 7th arg, rbp+24 is the 8th arg, etc.
                            int stack_arg_offset = 16 + (arg_idx - 6) * 8;
                            fprintf(out_file, "  movq %d(%%rbp), %%rax\n", stack_arg_offset);
                            fprintf(out_file, "  movq %%rax, %d(%%rbp)\n", offset);
                        }
                        arg_list = arg_list->next;
                        arg_idx++;
                    }
                }
                break;
            }

            case I_LEAVE: {
                fprintf(out_file, ".L_leave_%s:\n", instr->var1->id.name);
                // Function call epilogue
                if (strcmp(instr->var1->id.name, "main") == 0) {
                    fprintf(out_file, "  movq %%rax, %%rsi\n");  
                    fprintf(out_file, "  leaq fmt(%%rip), %%rdi\n");    
                    fprintf(out_file, "  movq $0, %%rax\n");             
                    fprintf(out_file, "  call printf\n");
                }
                fprintf(out_file, "  movq %%rbp, %%rsp\n");
                fprintf(out_file, "  popq %%rbp\n");
                fprintf(out_file, "  ret\n");
                for (int v = 0; v < var_count; ++v) {
                    // Free memory of variables used in this function
                    free(var_map[v].name);
                    var_map[v].name = NULL;
                }
                var_count = 0;
                break;
            }

            case I_RET: {
                char current_func_name[64] = "unknown";
                for(int j = i; j >= 0; j--) {
                    // Find the nearest preceding I_ENTER to get the function name
                    if (code[j].instruct->instruct.type_instruct == I_ENTER) {
                        strncpy(current_func_name, code[j].var1->id.name, sizeof(current_func_name)-1);
                        current_func_name[sizeof(current_func_name)-1] = '\0'; // Line end to avoid buffer overflow
                        break;
                    }
                }
                // Move return value to %rax and jump to function epilogue
                if(instr->var1) {
                    get_operand_str(instr->var1, op1, sizeof(op1));
                    fprintf(out_file, "  movq %s, %%rax\n", op1);
                }
                fprintf(out_file, "  jmp .L_leave_%s\n", current_func_name);
                break;
            }

            case I_LOADVAL:
                // Always used for loading literal values into the stack (doesn't work if you want to make memory -> memory moves)
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %s\n", op1, dest);
                break;

            case I_STORE:
                // Used for storing values into memory (assignments)
                // rax used for intermediate saving place because we can't make memory -> memory moves
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;

            // Arithmetic and logical operations are the same except for NEG
            case I_ADD: case I_SUB: case I_MUL: case I_AND: case I_OR:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                const char* op_str = (instr->instruct->instruct.type_instruct == I_ADD) ? "addq" :
                                     (instr->instruct->instruct.type_instruct == I_SUB) ? "subq" :
                                     (instr->instruct->instruct.type_instruct == I_MUL) ? "imulq" :
                                     (instr->instruct->instruct.type_instruct == I_AND) ? "andq" : "orq";
                // rax used for intermediate values
                fprintf(out_file, "  %s %s, %%rax\n", op_str, op2);
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;

            case I_DIV: case I_MOD:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  cqto\n"); // Sign-extends value in rax to the rdx:rax register pair (necessary to use idivq)
                fprintf(out_file, "  idivq %s\n", op2);
                char* result_reg = (instr->instruct->instruct.type_instruct == I_DIV) ? "%rax" : "%rdx";
                // idivq saves the result of the division in rax, and the remainder in rdx
                fprintf(out_file, "  movq %s, %s\n", result_reg, dest);
                break;

            case I_MIN:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  negq %%rax\n"); // "negates" the value in rax, use the two's complement operation
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;

            case I_NEG:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  testq %%rax, %%rax\n"); // If the value in rax was 0, this sets the ZF (cpu flag) in 1
                fprintf(out_file, "  sete %%al\n"); // If ZF is in 1, puts 1 in the al register (8 bits), otherwise it puts 0 in al
                fprintf(out_file, "  movzbq %%al, %%rax\n"); // Moves the value in al to rax filling all missing bits with 0's
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;

            case I_LES: case I_GRT: case I_EQ: case I_NEQ: case I_LEQ: case I_GEQ: {
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->var2, op2, sizeof(op2));
                get_operand_str(instr->reg, dest, sizeof(dest));
                const char* set_op;
                switch (instr->instruct->instruct.type_instruct) {
                    case I_LES: set_op = "setl"; break; case I_GRT: set_op = "setg"; break;
                    case I_EQ:  set_op = "sete"; break; case I_NEQ: set_op = "setne"; break;
                    case I_LEQ: set_op = "setle"; break; case I_GEQ: set_op = "setge"; break;
                    default: set_op = ""; break;
                }
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  cmpq %s, %%rax\n", op2); // Compares operands, sets CPU flags (Zero Flag, Sign Flag, etc.)
                fprintf(out_file, "  %s %%al\n", set_op); // Checks CPU flags and sets result in al based on the prevoius comparation
                fprintf(out_file, "  movzbq %%al, %%rax\n");
                fprintf(out_file, "  movq %%rax, %s\n", dest);
                break;
            }

            case I_LABEL:
                get_operand_str(instr->var1, op1, sizeof(op1));
                fprintf(out_file, "%s:\n", op1);
                break;

            case I_JMP:
                get_operand_str(instr->var1, op1, sizeof(op1));
                fprintf(out_file, "  jmp %s\n", op1);
                break;

            case I_JMPF:
                get_operand_str(instr->var1, op1, sizeof(op1));
                get_operand_str(instr->reg, dest, sizeof(dest));
                fprintf(out_file, "  movq %s, %%rax\n", op1);
                fprintf(out_file, "  testq %%rax, %%rax\n"); // If value in rax is 0, this sets Zero Flag in 1
                fprintf(out_file, "  jz %s\n", dest); // Jump if Zero Flag is 1
                break;

            case I_PARAM:
                // First 6 parameters go in registers, rest go on stack
                get_operand_str(instr->var1, op1, sizeof(op1));
                if (param_count < 6) {
                    fprintf(out_file, "  movq %s, %s\n", op1, arg_regs[param_count]);
                } else {
                    // Parameters beyond the 6th need to be pushed onto stack
                    // We'll collect them and push in reverse order before the call
                    fprintf(out_file, "  pushq %s\n", op1);
                    stack_params++;
                }
                param_count++;
                break;

            case I_CALL:
                fprintf(out_file, "  call %s\n", instr->var1->id.name);
                if (instr->reg) {
                    // Saves returned value
                    get_operand_str(instr->reg, dest, sizeof(dest));
                    fprintf(out_file, "  movq %%rax, %s\n", dest);
                }
                // Clean up stack parameters (arguments 7+)
                if (stack_params > 0) {
                    fprintf(out_file, "  addq $%d, %%rsp\n", stack_params * 8);
                }
                param_count = 0;
                stack_params = 0;
                break;

            case I_LOAD:
                break;

            default:
                fprintf(out_file, "  # Unknown instruction\n");
                break;
        }
    }
}