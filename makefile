all: control writer

control: control.o
	gcc control.o -o $@

writer: writer.o
	gcc writer.o -o $@

writer.o: writer.c shared.h
	gcc -c writer.c

control.o: control.c shared.h
	gcc -c control.c

clean:
	rm *.o writer control
