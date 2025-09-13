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

static void print_leaf(AST_NODE *n) {
    switch (n->leaf_type) {
        case TYPE_INT:
            printf("INT(%d)", n->value->int_leaf.value);
            break;
        case TYPE_BOOL:
            printf("BOOL(%s)", n->value->bool_leaf.value ? "true" : "false");
            break;
        case TYPE_ID:
            printf("ID(%s)", n->value->id_leaf->id_name);
            break;
        default:
            printf("LEAF(?)");
            break;
    }
}

void print_node(AST_NODE *node, const char *prefix, int is_last) {
    if (!node) return;

    printf("%s", prefix);
    printf(is_last ? "└── " : "├── ");

    if (node->is_leaf) {
        print_leaf(node);
    } else {
        printf("%s", op_to_string(node->op));
    }
    printf("\n");

    char new_prefix[512];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    AST_NODE *children[2];
    int count = 0;
    if (node->left) children[count++] = node->left;
    if (node->arity == BINARY && node->right) children[count++] = node->right;

    for (int i = 0; i < count; ++i) {
        print_node(children[i], new_prefix, i == count - 1);
    }
}

void print_program_horizontal(void) {
    printf("=== Program AST ===\n");
    AST_ROOT *cur = head_ast;
    int idx = 0;
    while (cur) {
        printf("Statement %d\n", idx++);
        print_node(cur->sentence, "", 1);
        cur = cur->next;
    }
    printf("========================\n");
}

static void print_spaces(int n) {
    for (int i = 0; i < n; ++i) putchar(' ');
}

void print_tree_ascii(AST_NODE *node, int depth, int is_left) {
    if (!node) return;

    if (node->arity == BINARY && node->right) {
        print_tree_ascii(node->right, depth + 1, 0);
    }

    print_spaces(depth * 5);
    if (depth > 0) {
        printf(is_left ? "/----" : "\\----");
    }

    if (node->is_leaf) {
        switch (node->leaf_type) {
            case TYPE_INT:
                printf("%d\n", node->value->int_leaf.value);
                break;
            case TYPE_BOOL:
                printf("%s\n", node->value->bool_leaf.value ? "true" : "false");
                break;
            case TYPE_ID:
                printf("%s\n", node->value->id_leaf->id_name);
                break;
            default:
                printf("?\n");
                break;
        }
    } else {
        printf("%s\n", op_to_string(node->op));
    }

    if (node->left) {
        print_tree_ascii(node->left, depth + 1, 1);
    }
}

void print_program_vertical(void) {
    printf("=== Program AST ===\n");
    AST_ROOT *cur = head_ast;
    int idx = 0;
    while (cur) {
        printf("\nStatement %d\n\n", idx++);
        print_tree_ascii(cur->sentence, 0, 0);
        cur = cur->next;
    }
    printf("\n========================\n\n");
}


static const char *id_type_to_string(ID_TYPE t) {
    switch (t) {
        case CONST_INT:  return "CONST_INT";
        case CONST_BOOL: return "CONST_BOOL";
        case UNKNOWN:    return "UNKNOWN";
        default:         return "?";
    }
}

void print_id_table(void) {
    printf("\n=== Symbol Table (ID_TABLE) ===\n");
    if (!head_table) {
        printf("(empty)\n");
        printf("================================\n");
        return;
    }

    // First: linear chain overview
    printf("Chain:\n  ");
    ID_TABLE *cur = head_table;
    size_t idx = 0;
    while (cur) {
        printf("[%zu:%s]", idx, cur->id_name ? cur->id_name : "(null)");
        if (cur->next) printf(" -> ");
        cur = cur->next;
        idx++;
    }
    printf(" -> NULL\n\n");

    // Detailed table header
    printf("+-----+-----------------+-----------------+-------------+---------------+-----------------+\n");
    printf("| idx | self            | id_name         | id_type     | value         | next            |\n");
    printf("+-----+-----------------+-----------------+-------------+---------------+-----------------+\n");

    cur = head_table;
    idx = 0;
    while (cur) {
        char value_buf[64];
        if (!cur->data) {
            snprintf(value_buf, sizeof value_buf, "%s", "(null)");
        } else {
            switch (cur->id_type) {
                case CONST_INT:
                    snprintf(value_buf, sizeof value_buf, "%d", *(int*)cur->data);
                    break;
                case CONST_BOOL:
                    snprintf(value_buf, sizeof value_buf, "%s", (*(int*)cur->data) ? "true" : "false");
                    break;
                case UNKNOWN:
                default:
                    snprintf(value_buf, sizeof value_buf, "%s", "?");
                    break;
            }
        }

        printf("| %3zu | %14p | %-15s | %-11s | %-13s | %14p |\n",
               idx,
               (void*)cur,
               cur->id_name ? cur->id_name : "(null)",
               id_type_to_string(cur->id_type),
               value_buf,
               (void*)cur->next);

        cur = cur->next;
        idx++;
    }
    printf("+-----+-----------------+-----------------+-------------+---------------+-----------------+\n\n");

    // Per-node verbose block
    printf("Verbose nodes:\n");
    cur = head_table;
    idx = 0;
    while (cur) {
        printf("Node %zu @ %p\n", idx, (void*)cur);
        printf("  id_name : %s\n", cur->id_name ? cur->id_name : "(null)");
        printf("  id_type : %s (%d)\n", id_type_to_string(cur->id_type), cur->id_type);
        if (cur->data) {
            switch (cur->id_type) {
                case CONST_INT:
                    printf("  data    : (int) %d (addr=%p)\n", *(int*)cur->data, cur->data);
                    break;
                case CONST_BOOL:
                    printf("  data    : (bool) %s (addr=%p)\n",
                           (*(int*)cur->data) ? "true" : "false", cur->data);
                    break;
                default:
                    printf("  data    : (unknown) addr=%p\n", cur->data);
            }
        } else {
            printf("  data    : (null)\n");
        }
        printf("  next    : %p\n", (void*)cur->next);
        printf("------------------------------------------------------------\n");
        cur = cur->next;
        idx++;
    }
    printf("================================\n");
}