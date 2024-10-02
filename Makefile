PROG1 = procpi

CC = gcc
CFLAGS = -lm


all: $(PROG1)

$(PROG1): $(PROG1).c
	@echo "\n[+] Compilando el programa CON multiprocesos $(PROG1).c\n"
	$(CC) -o $(PROG1) $(PROG1).c $(CFLAGS)

clean:
	@echo "Limpiando archivos generados..."
	rm -f $(PROG1)