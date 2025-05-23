CC = gcc
CFLAGS = -Wall -Wextra -Iparser -Ievaluator -Imodels -g
AR = ar
ARFLAGS = rcs

LIB_BIN = libs
PARSER_SRC = parser
EVALUATOR_SRC = evaluator
TARGET = li.out

.PHONY = all clean

all: $(LIB_BIN)/libparser.a $(LIB_BIN)/libevaluator.a main.o
	$(CC) main.o -L$(LIB_BIN) -lparser -levaluator -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_BIN):
	mkdir -p $(LIB_BIN)

$(LIB_BIN)/libparser.a: $(wildcard $(PARSER_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(PARSER_SRC)/*.c 
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o 

$(LIB_BIN)/libevaluator.a: $(wildcard $(EVALUATOR_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(EVALUATOR_SRC)/*.c
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o

clean:
	rm -f $(TARGET)
	rm -f $(PARSER_SRC)/*.o $(EVALUATOR_SRC)/*.o
	rm -rf $(LIB_BIN)