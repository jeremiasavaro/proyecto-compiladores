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
        case OP_DECL_INT:        return "decl int";
        case OP_DECL_BOOL:       return "decl bool";
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
                    if (node->leaf.value && node->leaf.value->id_leaf)
                        snprintf(buf, bufsz, "%s", node->leaf.value->id_leaf);
                    else
                        snprintf(buf, bufsz, "ID(?)");
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
        case AST_METHOD:
            if (node->method.block)
                snprintf(buf, bufsz, "METHOD %s", node->method.name ? node->method.name : "(null)");
            else
                snprintf(buf, bufsz, "CALL %s", node->method.name ? node->method.name : "(null)");
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
    } else if (node->type == AST_METHOD) {
        /* Print method arguments as a grouped 'ARGS' node (so args appear as
           direct children of METHOD), then print the BLOCK. */
        if (node->method.args) {
            AST_NODE_LIST *it = node->method.args;
            int total = 0; for (AST_NODE_LIST *t = it; t; t = t->next) total++;
            int has_block = node->method.block ? 1 : 0;
            // print ARGS group (not last if there is a block)
            printf("%s%s%s\n", new_prefix, (has_block ? "├── " : "└── "), "ARGS");
            char args_prefix[512];
            snprintf(args_prefix, sizeof(args_prefix), "%s%s", new_prefix, (has_block ? "│   " : "    "));
            int ai = 0;
            AST_NODE_LIST *it2 = node->method.args;
            while (it2) {
                AST_NODE *arg = it2->first;
                // print arg name as a simple label (assume TYPE_ID leaf)
                if (arg && arg->type == AST_LEAF && arg->leaf.leaf_type == TYPE_ID) {
                    char namebuf[256];
                    if (arg->leaf.value && arg->leaf.value->id_leaf)
                        snprintf(namebuf, sizeof(namebuf), "%s", arg->leaf.value->id_leaf);
                    else
                        snprintf(namebuf, sizeof(namebuf), "(arg?)");
                    printf("%s%s%s\n", args_prefix, (ai == total - 1 && !has_block ? "└── " : "├── "), namebuf);
                } else {
                    // fallback: print the node normally
                    print_node_tree(arg, args_prefix, ai == total - 1 && !has_block);
                }
                it2 = it2->next; ai++;
            }
        }
        if (node->method.block) {
            // block as last child
            print_node_tree(node->method.block, new_prefix, 1);
        }
    } else if (node->type == AST_BLOCK) {
        AST_NODE_LIST *it = node->block.stmts;
        int total = 0; for (AST_NODE_LIST *t = it; t; t = t->next) total++;
        if (total == 0) return;
        AST_NODE **arr = malloc(sizeof(AST_NODE*) * total);
        if (!arr) {
            AST_NODE_LIST *tmp = it;
            while (tmp) {
                print_node_tree(tmp->first, new_prefix, tmp->next == NULL);
                tmp = tmp->next;
            }
            return;
        }
        int i = 0;
        for (AST_NODE_LIST *t = it; t; t = t->next) {
            arr[i++] = t->first;
        }
        /* Separate declarations (OP_DECL_*) from other statements so we can
           display them grouped under a synthetic "DECLS" node inside the block. */
        AST_NODE **decls = malloc(sizeof(AST_NODE*) * total);
        AST_NODE **stmts = malloc(sizeof(AST_NODE*) * total);
        int d = 0, s = 0;
        for (int j = total - 1; j >= 0; --j) {
            AST_NODE *n = arr[j];
            if (n && n->type == AST_COMMON && (n->common.op == OP_DECL_INT || n->common.op == OP_DECL_BOOL)) {
                decls[d++] = n;
            } else {
                stmts[s++] = n;
            }
        }

        int groups = (d>0?1:0) + (s>0?1:0);
        int group_index = 0;
        // print DECLS group if any
        if (d > 0) {
            int is_last_group = (group_index == groups - 1);
            char decl_prefix[512];
            snprintf(decl_prefix, sizeof(decl_prefix), "%s%s", new_prefix, (is_last_group ? "    " : "│   "));
            for (int k = 0; k < d; ++k) {
                // last decl in group is last if group is last
                int is_last_decl = (k == d - 1) && is_last_group;
                print_node_tree(decls[k], decl_prefix, is_last_decl);
            }
            group_index++;
        }
        // print STATEMENTS group if any
        if (s > 0) {
            int is_last_group = (group_index == groups - 1);
            if (groups > 1) {
                printf("%s%s%s\n", new_prefix, (is_last_group ? "└── " : "├── "), "STATEMENTS");
            }
            char stm_prefix[512];
            snprintf(stm_prefix, sizeof(stm_prefix), "%s%s", new_prefix, (is_last_group ? "    " : "│   "));
            for (int k = 0; k < s; ++k) {
                int is_last_stmt = (k == s - 1) && is_last_group;
                print_node_tree(stmts[k], stm_prefix, is_last_stmt);
            }
        }
        free(arr);
        free(decls);
        free(stmts);
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
    if (!root) { printf("(empty program)\n========================\n"); return; }
    /* The AST_ROOT list may be built with the newest sentences at the head.
       To print top-to-bottom (the order they appear in the source), collect
       nodes into an array and iterate from first to last. */
    int total = 0;
    for (AST_ROOT *t = root; t; t = t->next) total++;
    AST_NODE **arr = malloc(sizeof(AST_NODE*) * total);
    if (!arr) {
        // fallback: print in stored order
        AST_ROOT *cur = root; int idx = 0;
        while (cur) {
            printf("Statement %d\n", idx++);
            print_node_tree(cur->sentence, "", 1);
            cur = cur->next;
        }
        printf("========================\n");
        return;
    }
    int i = 0;
    for (AST_ROOT *t = root; t; t = t->next) {
        arr[i++] = t->sentence;
    }
    // stored list is head-most-recent; print from arr[total-1]..arr[0]
    int idx = 0;
    for (int j = total - 1; j >= 0; --j) {
        printf("\nStatement %d\n", idx++);
        print_node_tree(arr[j], "", 1);
    }
    free(arr);
    printf("========================\n");
}