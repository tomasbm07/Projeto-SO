cc		 = gcc
CFLAGS	 = -Wall -g -lpthread -D_REENTRANT
LDFLAGS  =
DEPS	 = main.c main.h teste.c
OBJFILES = main.o teste.o
TARGET	 = projeto.sh

all: $(TARGET)

$(TARGET): $(OBJFILES) $(DEPS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~