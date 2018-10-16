BIN= prj2
OBJS= channel.o queue.o
INCS= channel.h messages.h

all: prj2

prj2: main.c $(OBJS) $(INCS)
	gcc -pg -g \
		main.c \
		$(OBJS) \
		-o prj2

channel.o: channel.c $(INCS)
	gcc -c -g -pg channel.c

queue.o: queue.c $(INCS)
	gcc -c -g -pg queue.c

clean:
	rm -f $(BIN) $(OBJS) \
		gmon.out
