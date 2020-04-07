
OBJECTS=bbutest bbutest_disarmbbu

EXTRA_CFLAGS := 

CC=cc

.c.o:
	  $(CC) $(CFLAGS) $(INCLUDES) $(EXTRA_CFLAGS) -c -g $<

all: $(OBJECTS) 
	  $(CC)  $<.c -o $<
	 

clean: $(OBJECTS) 
	  rm -f $< *.o 
