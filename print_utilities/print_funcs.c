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
    switch (node->type) {
        case AST_LEAF:
            if (!node->leaf.value) { snprintf(buf, bufsz, "LEAF(NULL)"); return; }
            switch (node->leaf.leaf_type) {
                case TYPE_INT:
                    snprintf(buf, bufsz, "%d", node->leaf.value->int_leaf.value);
                    return;
                case TYPE_BOOL:
                    snprintf(buf, bufsz, "%s", node->leaf.value->bool_leaf.value ? "true" : "false");
                    return;
                case TYPE_ID:
                    if (node && node->leaf.value && node->leaf.value->id_leaf && node->leaf.value->id_leaf->id_name) {
                        snprintf(buf, bufsz, "%s", node->leaf.value->id_leaf->id_name);
                    } else {
                        snprintf(buf, bufsz, "ID(?)");
                    }
                    return;
                default:
                    snprintf(buf, bufsz, "LEAF(?)");
                    return;
            }
            break;
        case AST_COMMON:
            snprintf(buf, bufsz, "%s", op_to_string(node->common.op));
            return;
        case AST_IF:
            snprintf(buf, bufsz, "IF");
            return;
        case AST_WHILE:
            snprintf(buf, bufsz, "WHILE");
            return;
        case AST_METHOD_DECL:
            if (node->method_decl.block)
                snprintf(buf, bufsz, "METHOD %s", node->method_decl.name ? node->method_decl.name : "(null)");
            else if (node->method_decl.is_extern)
                snprintf(buf, bufsz, "EXTERN METHOD %s", node->method_decl.name ? node->method_decl.name : "(null)");
            else
                snprintf(buf, bufsz, "CALL %s", node->method_decl.name ? node->method_decl.name : "(null)");
            return;
        case AST_METHOD_CALL:
            snprintf(buf, bufsz, "CALL %s", node->method_call.name ? node->method_call.name : "(null)");
            return;
        case AST_BLOCK:
            snprintf(buf, bufsz, "BLOCK");
            return;
        case AST_NULL:
            snprintf(buf, bufsz, "NULL");
            return;
        default:
            snprintf(buf, bufsz, "?(type=%d)", node->type);
            return;
    }
}

// recursive tree printer using ASCII connectors
static void print_node_tree(AST_NODE *node, const char *prefix, int is_last) {
    if (!node) return;
    char label[256];
    node_label(node, label, sizeof(label));

    printf("%s%s%s\n", prefix, (is_last ? "└── " : "├── "), label);

    // build new prefix
    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, (is_last ? "    " : "│   "));

    // handle children depending on node type
    if (node->type == AST_COMMON) {
        AST_NODE *children[2];
        int count = 0;
        if (node->common.left) children[count++] = node->common.left;
        if (node->common.arity == BINARY && node->common.right) children[count++] = node->common.right;
        for (int i = 0; i < count; ++i) {
            print_node_tree(children[i], new_prefix, i == count - 1);
        }
    } else if (node->type == AST_IF) {
        AST_NODE *children[3]; int count = 0;
        if (node->if_stmt.condition) children[count++] = node->if_stmt.condition;
        if (node->if_stmt.then_block) children[count++] = node->if_stmt.then_block;
        if (node->if_stmt.else_block) children[count++] = node->if_stmt.else_block;
        for (int i = 0; i < count; ++i) print_node_tree(children[i], new_prefix, i == count - 1);
    } else if (node->type == AST_WHILE) {
        AST_NODE *children[2]; int count = 0;
        if (node->while_stmt.condition) children[count++] = node->while_stmt.condition;
        if (node->while_stmt.block) children[count++] = node->while_stmt.block;
        for (int i = 0; i < count; ++i) print_node_tree(children[i], new_prefix, i == count - 1);
    } else if (node->type == AST_METHOD_DECL) {
        /* Print method arguments as a grouped 'ARGS' node (so args appear as
           direct children of METHOD), then print the BLOCK. */
        if (node->method_decl.args) {
            AST_NODE_LIST *it = node->method_decl.args;
            int total = 0; for (AST_NODE_LIST *t = it; t; t = t->next) total++;
            int has_block = node->method_decl.block ? 1 : 0;
            // print ARGS group (not last if there is a block)
            printf("%s%s%s\n", new_prefix, (has_block ? "├── " : "└── "), "ARGS");
            char args_prefix[512];
            snprintf(args_prefix, sizeof(args_prefix), "%s%s", new_prefix, (has_block ? "│   " : "    "));
            int ai = 0;
            AST_NODE_LIST *it2 = node->method_decl.args;
            while (it2) {
                AST_NODE *arg = it2->first;
                // print arg name as a simple label (assume TYPE_ID leaf)
                if (arg && arg->type == AST_LEAF && arg->leaf.leaf_type == TYPE_ID) {
                    char namebuf[256];
                    if (arg->leaf.value && arg->leaf.value->id_leaf && arg->leaf.value->id_leaf->id_name) {
                        snprintf(namebuf, sizeof(namebuf), "%s", arg->leaf.value->id_leaf->id_name);
                    } else {
                        snprintf(namebuf, sizeof(namebuf), "(arg?)");
                    }
                    printf("%s%s%s\n", args_prefix, (ai == total - 1 && !has_block ? "└── " : "├── "), namebuf);
                } else {
                    // fallback: print the node normally
                    print_node_tree(arg, args_prefix, ai == total - 1 && !has_block);
                }
                it2 = it2->next; ai++;
            }
        }
        if (node->method_decl.block) {
            // block as last child
            print_node_tree(node->method_decl.block, new_prefix, 1);
        }

    } else if (node->type == AST_METHOD_CALL) {
        if (node->method_call.args) {
            printf("%s└── ARGS\n", new_prefix);
            char args_prefix[512];
            snprintf(args_prefix, sizeof(args_prefix), "%s    ", new_prefix);

            int total = 0;
            for (AST_NODE_LIST *t = node->method_call.args; t; t = t->next) total++;

            int ai = 0;
            for (AST_NODE_LIST *it = node->method_call.args; it; it = it->next, ai++) {
                print_node_tree(it->first, args_prefix, ai == total - 1);
            }
        }
    } else if (node->type == AST_BLOCK) {
        int total = 0;
        for (AST_NODE_LIST *it = node->block.stmts; it; it = it->next)
            total++;
        int i = 0;
        for (AST_NODE_LIST *it = node->block.stmts; it; it = it->next, i++) {
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

static const char* id_type_to_str(ID_TYPE t) {
    switch (t) {
        case CONST_INT: return "int";
        case CONST_BOOL: return "bool";
        case METHOD: return "method";
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
            printf("  - Name: %s | Type: %s", id->id_name, id_type_to_str(id->id_type));
            if (id->id_type == METHOD) {
                printf(" | Return: %s | Args: %d\n", return_type_to_str(id->method.return_type), id->method.num_args);
                ARGS_LIST* arg = id->method.arg_list;
                int arg_idx = 0;
                while (arg) {
                    printf("      Arg %d: %s (%s)\n", arg_idx++, arg->arg->name, id_type_to_str(arg->arg->type));
                    arg = arg->next;
                }
            } else {
                printf("\n");
            }
        }
        printf("\n");
    }
}