CC=gcc
CFLAGS="-Wall" "-fPIC" "-shared" "-ldl"
OUTNAME=h0h0

all: $(OUTNAME).so

$(OUTNAME).so:
	$(CC) $(CFLAGS) -o $(OUTNAME).so $(OUTNAME).c

clean:
	rm -f *.so
