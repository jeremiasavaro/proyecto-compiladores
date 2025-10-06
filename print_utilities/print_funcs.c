#include <stdio.h>
#include <string.h>
#include "print_funcs.h"

static const char *op_to_string(OPERATOR op) {
    switch (op) {
        case OP_ADDITION:        return "+";
        case OP_SUBTRACTION:     return "-";
        case OP_MULTIPLICATION:  return "*";
        case OP_DIVISION:        return "/";
        case OP_MOD:             return "%";
        case OP_LES:             return "<";
        case OP_GRT:             return ">";
        case OP_EQ:              return "==";
        case OP_NEQ:             return "!=";
        case OP_LEQ:             return "<=";
        case OP_GEQ:             return ">=";
        case OP_MINUS:           return "UMINUS";
        case OP_AND:             return "&&";
        case OP_OR:              return "||";
        case OP_NEG:             return "!";
        case OP_ASSIGN:          return "=";
        case OP_RETURN:          return "return";
        case OP_DECL:       return "decl";
        default:                 return "?";
    }
}

static void node_label(AST_NODE *node, char *buf, size_t bufsz) {
    if (!node) { snprintf(buf, bufsz, "(null)"); return; }
    switch (node->info->type) {
        case AST_LEAF:
            if (!node->info->leaf.value) { snprintf(buf, bufsz, "LEAF(NULL)"); return; }
            switch (node->info->leaf.type) {
                case TYPE_INT:
                    snprintf(buf, bufsz, "%d", node->info->leaf.value->int_value);
                    return;
                case TYPE_BOOL:
                    snprintf(buf, bufsz, "%s", node->info->leaf.value->bool_value ? "true" : "false");
                    return;
                case TYPE_ID:
                    if (node && node->info->leaf.value && node->info->leaf.value->id_leaf && node->info->leaf.value->id_leaf->info->id.name) {
                        snprintf(buf, bufsz, "%s", node->info->leaf.value->id_leaf->info->id.name);
                    } else {
                        snprintf(buf, bufsz, "ID(?)");
                    }
                    return;
                default:
                    snprintf(buf, bufsz, "LEAF(?)");
                    return;
            }
        case AST_COMMON:
            snprintf(buf, bufsz, "%s", op_to_string(node->info->common.op));
            return;
        case AST_IF:
            snprintf(buf, bufsz, "IF");
            return;
        case AST_WHILE:
            snprintf(buf, bufsz, "WHILE");
            return;
        case AST_METHOD_DECL:
            if (node->info->method_decl.block)
                snprintf(buf, bufsz, "METHOD %s", node->info->method_decl.name ? node->info->method_decl.name : "(null)");
            else if (node->info->method_decl.is_extern)
                snprintf(buf, bufsz, "EXTERN METHOD %s", node->info->method_decl.name ? node->info->method_decl.name : "(null)");
            else
                snprintf(buf, bufsz, "CALL %s", node->info->method_decl.name ? node->info->method_decl.name : "(null)");
            return;
        case AST_METHOD_CALL:
            snprintf(buf, bufsz, "CALL %s", node->info->method_call.name ? node->info->method_call.name : "(null)");
            return;
        case AST_BLOCK:
            snprintf(buf, bufsz, "BLOCK");
            return;
        case AST_NULL:
            snprintf(buf, bufsz, "NULL");
            return;
        default:
            snprintf(buf, bufsz, "?(type=%d)", node->info->type);
    }
}

// recursive tree printer using ASCII connectors
static void print_node_tree(AST_NODE *node, const char *prefix, int is_last) {
    if (!node) return;
    char label[256];
    node_label(node, label, sizeof(label));

    printf("%s%s%s\n", prefix, is_last ? "└── " : "├── ", label);

    // build new prefix
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // handle children depending on node type
    if (node->info->type == AST_COMMON) {
        AST_NODE *children[2];
        int count = 0;
        if (node->info->common.left) children[count++] = node->info->common.left;
        if (node->info->common.arity == BINARY && node->info->common.right) children[count++] = node->info->common.right;
        for (int i = 0; i < count; ++i) {
            print_node_tree(children[i], new_prefix, i == count - 1);
        }
    } else if (node->info->type == AST_IF) {
        AST_NODE *children[3]; int count = 0;
        if (node->info->if_stmt.condition) children[count++] = node->info->if_stmt.condition;
        if (node->info->if_stmt.then_block) children[count++] = node->info->if_stmt.then_block;
        if (node->info->if_stmt.else_block) children[count++] = node->info->if_stmt.else_block;
        for (int i = 0; i < count; ++i) print_node_tree(children[i], new_prefix, i == count - 1);
    } else if (node->info->type == AST_WHILE) {
        AST_NODE *children[2]; int count = 0;
        if (node->info->while_stmt.condition) children[count++] = node->info->while_stmt.condition;
        if (node->info->while_stmt.block) children[count++] = node->info->while_stmt.block;
        for (int i = 0; i < count; ++i) print_node_tree(children[i], new_prefix, i == count - 1);
    } else if (node->info->type == AST_METHOD_DECL) {
        /* Print method arguments as a grouped 'ARGS' node (so args appear as
           direct children of METHOD), then print the BLOCK. */
        if (node->info->method_decl.args) {
            ARGS_LIST *it = node->info->method_decl.args;
            int total = 0;
            for (ARGS_LIST *t = it; t; t = t->next) total++;
            int has_block = node->info->method_decl.block ? 1 : 0;
            // print ARGS group (not last if there is a block)
            printf("%s%s%s\n", new_prefix, has_block ? "├── " : "└── ", "ARGS");
            char args_prefix[512];
            snprintf(args_prefix, sizeof(args_prefix), "%s%s", new_prefix, has_block ? "│   " : "    ");
            int ai = 0;
            ARGS_LIST *it2 = node->info->method_decl.args;
            while (it2) {
                ARGS *arg = it2->arg;
                char namebuf[256];
                if (arg && arg->name) {
                    const char* type_str = arg->type == TYPE_INT ? "int" : "bool";
                    snprintf(namebuf, sizeof(namebuf), "%s (%s)", arg->name, type_str);
                } else {
                    snprintf(namebuf, sizeof(namebuf), "(arg?)");
                }
                printf("%s%s%s\n", args_prefix, ai == total - 1 && !has_block ? "└── " : "├── ", namebuf);
                it2 = it2->next;
                ai++;
            }
        }

        if (node->info->method_decl.block) {
            // block as last child
            print_node_tree(node->info->method_decl.block, new_prefix, 1);
        }

    } else if (node->info->type == AST_METHOD_CALL) {
        if (node->info->method_call.args) {
            printf("%s└── ARGS\n", new_prefix);
            char args_prefix[512];
            snprintf(args_prefix, sizeof(args_prefix), "%s    ", new_prefix);

            int total = 0;
            for (AST_NODE_LIST *t = node->info->method_call.args; t; t = t->next) total++;

            int ai = 0;
            for (AST_NODE_LIST *it = node->info->method_call.args; it; it = it->next, ai++) {
                print_node_tree(it->first, args_prefix, ai == total - 1);
            }
        }
    } else if (node->info->type == AST_BLOCK) {
        int total = 0;
        for (AST_NODE_LIST *it = node->info->block.stmts; it; it = it->next)
            total++;
        int i = 0;
        for (AST_NODE_LIST *it = node->info->block.stmts; it; it = it->next, i++) {
            print_node_tree(it->first, new_prefix, i == total - 1);
        }
    }
}

void print_ast_node(AST_NODE *node, int indent) {
    // build a prefix of spaces
    char prefix[128] = {0};
    int n = indent * 4;
    if (n >= (int)sizeof(prefix)) n = sizeof(prefix) - 1;
    for (int i = 0; i < n; ++i) prefix[i] = ' ';
    prefix[n] = '\0';
    print_node_tree(node, prefix, 1);
}

void print_ast_list(AST_NODE_LIST *list, int indent) {
    if (!list) return;
    int total = 0; for (AST_NODE_LIST *t = list; t; t = t->next) total++;
    int i = 0;
    while (list) {
        // provide indentation as prefix
        char prefix[128] = {0};
        int n = indent * 4;
        if (n >= (int)sizeof(prefix)) n = sizeof(prefix) - 1;
        for (int k = 0; k < n; ++k) prefix[k] = ' ';
        prefix[n] = '\0';
        print_node_tree(list->first, prefix, i == total - 1);
        list = list->next; i++;
    }
}

void print_full_ast(AST_ROOT *root) {
    printf("=== Program AST ===\n");
    if (!root) {
        printf("(empty program)\n========================\n");
        return;
    }
    int idx = 0;
    for (AST_ROOT *cur = root; cur; cur = cur->next) {
        printf("\nStatement %d\n", idx++);
        print_node_tree(cur->sentence, "", 1);
    }
    printf("========================\n");
}

static const char* id_type_to_str(ID_TABLE* t) {

    switch (t->info->type) {
        case AST_METHOD_DECL: return "method";
        case TABLE_ID: {
            if (t->info->id.type == TYPE_INT) return "int";
            if (t->info->id.type == TYPE_BOOL) return "bool";
        }
        default: return "unknown";
    }
}

static const char* arg_type_to_str(TYPE t) {
    switch (t) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        default: return "unknown";
    }
}

static const char* return_type_to_str(RETURN_TYPE t) {
    switch (t) {
        case RETURN_INT: return "int";
        case RETURN_BOOL: return "bool";
        case RETURN_VOID: return "void";
        default: return "unknown";
    }
}

void print_symbol_table(TABLE_STACK* top) {
    int scope_level = 0;
    for (TABLE_STACK* scope = top; scope; scope = scope->up, scope_level++) {
        printf("Scope Level %d:\n", scope_level);
        for (ID_TABLE* id = scope->head_block; id; id = id->next) {
            printf("  - Name: %s | Type: %s", id->info->id.name, id_type_to_str(id));
            if (id->info->type == AST_METHOD_DECL) {
                printf(" | Return: %s | Args: %d\n", return_type_to_str(id->info->method_decl.return_type), id->info->method_decl.num_args);
                ARGS_LIST* arg = id->info->method_decl.args;
                int arg_idx = 0;
                while (arg) {
                    printf("      Arg %d: %s (%s)\n", arg_idx++, arg->arg->name, arg_type_to_str(arg->arg->type));
                    arg = arg->next;
                }
            } else {
                printf("\n");
            }
        }
        printf("\n");
    }
}