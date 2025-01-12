
CC=gcc
CFLAGS=-g -fsanitize=address
LDFLAGS=-lm -lraylib
EXECUTABLE=colorworms
AR = ar
ARFLAGS = rcs

LIBRARY_SRC = worms_effect.c
LIBRARY_OBJ = $(LIBRARY_SRC:.c=.o)
LIBRARY_NAME = libworms_effect.a

MAIN_SRC = main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
EXECUTABLE = colorworms

# Цель по умолчанию
all: $(LIBRARY_NAME) $(EXECUTABLE)

# Сборка статической библиотеки
$(LIBRARY_NAME): $(LIBRARY_OBJ)
	@echo [AR] $@
	@$(AR) $(ARFLAGS) $@ $^

# Сборка исполняемого файла, с использованием библиотеки
$(EXECUTABLE): $(MAIN_OBJ) $(LIBRARY_NAME)
	@echo [LINK] $@
	@$(CC) $(MAIN_OBJ) $(LIBRARY_NAME) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	@rm -f *.o
	@rm -f $(EXECUTABLE)
