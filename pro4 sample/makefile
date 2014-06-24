
RELEASE_FLAGS= -Wall -O3 -pipe
DEBUG_FLAGS = -Wall -g -I./inc
FLAGS = $(DEBUG_FLAGS) 
OUTDIR = ./out

OBJECTS = $(OUTDIR)/comm.o $(OUTDIR)/protocol_pro4.o


$(OUTDIR)/rov_sample: rov_sample.c $(OBJECTS)	
	$(CC) $(FLAGS) -O3 -o $(OUTDIR)/rov_sample $(OBJECTS) rov_sample.c

$(OUTDIR)/protocol_pro4.o: libsrc/protocol_pro4.c
	$(CC) -c $(FLAGS) -O3 -o $(OUTDIR)/protocol_pro4.o libsrc/protocol_pro4.c
	
# Compile: create object files from C source files.
$(OUTDIR)/%.o: %.c	
	$(CC) -c $(FLAGS) -O3 $< -o $@ 

clean:	
	rm $(OUTDIR)/*

