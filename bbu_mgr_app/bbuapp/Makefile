
OBJECTS=bbuapp.o 

EXTRA_CFLAGS := $(EXTRA_CFLAGS) -DDEBUG -DLINUX

CC=cc

.c.o:
	  $(CC) $(CFLAGS) $(INCLUDES) $(EXTRA_CFLAGS) -c -g $<

bbuapp: $(OBJECTS) $(LIBS)
	  $(CC) -g -pthread -lrt bbuapp.o -o $@
	 # $(CC) -g $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LIBS) -o $@

clean:
	  rm -f bbuapp *.o 
