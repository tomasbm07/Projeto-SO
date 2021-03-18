CFLAGS = -Wall -lpthread -D_REENTRANT -g

#sempre que houver ficheiro novo, aidicionar .o á lista para ser compilado
OBJ = main.o   \
      link.h   \
	  teste.o  \
	  teste2.o \

%.o : %.c
	gcc -c $(CFLAGS) $< -o $@

projeto.sh: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)