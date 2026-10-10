CC = gcc
CFLAGS = -ggdb -Wall -Wextra -Werror -O3 -flto -ffast-math -pedantic -nostdlib
NAME = genitor
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
INC = inc/

all: build $(NAME)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -I$(INC) $< -c -o $@

build: 
	if [ ! -d "build/" ]; then \
		mkdir build; \
	fi

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INC) $(OBJ) -o $(NAME)

clean: 
	rm -rf build/

fclean: clean
	rm -rf $(NAME)

re: fclean all

.SILENT: build fclean clean
