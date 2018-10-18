BIN= prj2
OBJS= channel.o queue.o operation.o
INCS= channel.h messages.h operation.h

all: prj2

prj2: main.c $(OBJS) $(INCS)
	gcc -pg -g \
		main.c \
		$(OBJS) \
		-o prj2

channel.o: channel.c $(INCS)
	gcc -c -g -pg channel.c

operation.o: operation.c $(INCS)
	gcc -c -g -pg operation.c

queue.o: queue.c $(INCS)
	gcc -c -g -pg queue.c

clean:
	rm -f $(BIN) $(OBJS) \
		gmon.out
