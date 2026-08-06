CC = gcc
CFLAGS = -Wall -Wextra -Isrc
LDFLAGS = -lreadline



SRC    := $(shell find src -name '*.c')
OBJ     = $(SRC:.c=.o)
TARGET  = cyprsh

.PHONY: all debug release run valgrind clean fclean rebuild docs

# default build
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# debug
debug: CFLAGS += -DDEBUG -fsanitize=address,undefined -O0 -g -Werror 
debug: LDFLAGS += -fsanitize=address,undefined
debug: rebuild

# release
release: CFLAGS += -O2
release: rebuild

# run
run: all
	./$(TARGET)

# valgrind memory leak check
valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)

# clean object files
clean:
	find src -name "*.o" -delete

# clean everything
fclean: clean
	rm -f $(TARGET)

# rebuild
rebuild: fclean all

# docs
docs: 
	doxygen Doxyfile
