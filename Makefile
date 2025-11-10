# Variables
LEX     = flex
BISON   = bison
CC      = gcc
CFLAGS  = -g -Wall -Wextra -std=c11 -I. -Ierror_handling -Itree -Iprint_utilities -Isymbol_table -Iutils -Isemantic_analyzer -Iintermediate_code -Iobject_code -Ilibraries
TARGET  = ctds

# -O1 -fsanitize=address -fno-omit-frame-pointer    for debugging

# Sources
GEN_LEX_SRC   = lex.yy.c
GEN_Y_TAB_C   = parser.tab.c
GEN_Y_TAB_H   = parser.tab.h
LEX_FILE      = lex.l
YACC_FILE     = parser.y

SRCS = error_handling/error_handling.c tree/ast.c print_utilities/print_funcs.c symbol_table/symbol_table.c utils/utils.c utils/symbol.c semantic_analyzer/semantic_analyzer.c intermediate_code/intermediate_code.c intermediate_code/optimization.c object_code/object_code.c libraries/ctdsio.c main.c
OBJS = $(SRCS:.c=.o) $(GEN_LEX_SRC:.c=.o) $(GEN_Y_TAB_C:.c=.o)

.PHONY: all clean env prepare

all: prepare $(TARGET)

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

# Compile semantic_analyzer code
semantic_analyzer/%.o: semantic_analyzer/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile intermediate_code
intermediate_code/%.o: intermediate_code/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile object_code
object_code/%.o: object_code/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile 
libraries/%.o: libraries/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Export path so you can execute ctds without ./
env:
	@printf 'export PATH="$$PATH:%s"\n' "$(shell pwd)"

# Give execution permission to link script
prepare:
	@test -e link.sh || exit 0
	@test -x link.sh || chmod +x link.sh

clean:
	rm -f $(OBJS) $(TARGET) $(GEN_LEX_SRC) $(GEN_Y_TAB_C) $(GEN_Y_TAB_H)
	rm -f error_handling/*.o tree/*.o print_utilities/*.o symbol_table/*.o utils/*.o semantic_analyzer/*.o intermediate_code/*.o intermediate_code/*.codinter object_code/*.o object_code/*.s object_code/*.exe libraries/*.o
	rm -f tests/output/* *.output *.out tests/output_final *.exe
	rm -rf tests/output tests/output_executables tests/output_intermediate_code tests/output_object_code