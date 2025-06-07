CC = gcc
CFLAGS = -Wall -Wextra -Iparser -Ievaluator -Imodels -Icache -g
AR = ar
ARFLAGS = rcs

LIB_BIN = libs
MODELS_SRC = models
PARSER_SRC = parser
EVALUATOR_SRC = evaluator
CACHE_SRC = cache
TARGET = li.out

.PHONY = all clean

all: $(LIB_BIN)/libmodels.a $(LIB_BIN)/libparser.a $(LIB_BIN)/libcache.a $(LIB_BIN)/libevaluator.a main.o
	$(CC) main.o -L$(LIB_BIN) -lparser -levaluator -lmodels -lcache -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_BIN):
	mkdir -p $(LIB_BIN)

$(LIB_BIN)/libparser.a: $(wildcard $(PARSER_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(PARSER_SRC)/*.c 
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o 

$(LIB_BIN)/libmodels.a: $(wildcard $(MODELS_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(MODELS_SRC)/*.c 
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o 

$(LIB_BIN)/libcache.a: $(wildcard $(CACHE_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(CACHE_SRC)/*.c 
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o 

$(LIB_BIN)/libevaluator.a: $(wildcard $(EVALUATOR_SRC)/*.[ch]) | $(LIB_BIN)
	$(CC) $(CFLAGS) -c $(EVALUATOR_SRC)/*.c
	$(AR) $(ARFLAGS) $@ *.o
	rm -f *.o

clean:
	rm -f $(TARGET)
	rm -f $(PARSER_SRC)/*.o $(EVALUATOR_SRC)/*.o $(MODELS_SRC)/*.o
	rm -rf $(LIB_BIN)