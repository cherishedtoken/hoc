YFLAGS = -d
OBJS = hoc.c init.o math.o symbol.o

hoc:	$(OBJS)
	cc $(OBJS) -lm -o hoc

hoc.o: 	hoc.h

init.o symbol.o:	hoc.h y.tab.h

pr:
	@pr hoc.y hoc.h init.c math.c symbol.c makefile

clean:
	rm -f $(OBJS) y.tab.[ch]
