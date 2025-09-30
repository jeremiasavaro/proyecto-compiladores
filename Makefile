# Variables
LEX     = flex
BISON   = bison
CC      = gcc
CFLAGS  = -g -Wall -O1 -fsanitize=address -fno-omit-frame-pointer -Wextra -std=c11 -I. -Ierror_handling -Itree -Iprint_utilities -Isymbol_table -Iutils -Isemantic_analyzer
TARGET  = parser

# Sources
GEN_LEX_SRC   = lex.yy.c
GEN_Y_TAB_C   = parser.tab.c
GEN_Y_TAB_H   = parser.tab.h
LEX_FILE      = lex.l
YACC_FILE     = parser.y

SRCS = error_handling/error_handling.c tree/ast.c print_utilities/print_funcs.c symbol_table/symbol_table.c utils/utils.c semantic_analyzer/semantic_analyzer.c
OBJS = $(SRCS:.c=.o) $(GEN_LEX_SRC:.c=.o) $(GEN_Y_TAB_C:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(GEN_LEX_SRC) $(GEN_Y_TAB_C) $(GEN_Y_TAB_H) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Generate Bison files
$(GEN_Y_TAB_C) $(GEN_Y_TAB_H): $(YACC_FILE)
	$(BISON) -d -v $(YACC_FILE)

# Generate Flex file
$(GEN_LEX_SRC): $(LEX_FILE) $(GEN_Y_TAB_H)
	$(LEX) $(LEX_FILE)

# Generic compile rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile error_handling sources
error_handling/%.o: error_handling/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile tree sources
tree/%.o: tree/%.c
	$(CC) $(CFLAGS) -c $< -o $@

print_utilities/%.o: print_utilities/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile symbol_table sources
symbol_table/%.o: symbol_table/%.c
	$(CC) $(CFLAGS) -c $< -o $@

utils/%.o: utils/%.c
	$(CC) $(CFLAGS) -c $< -o $@

semantic_analyzer/%.o: semantic_analyzer/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(GEN_LEX_SRC) $(GEN_Y_TAB_C) $(GEN_Y_TAB_H)
	rm -f error_handling/*.o rm -f tree/*.o print_utilities/*.o symbol_table/*.o utils/*.o semantic_analyzer/*.o