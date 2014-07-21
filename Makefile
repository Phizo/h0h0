CC=gcc
CFLAGS="-Wall" "-fPIC" "-shared" "-ldl"
CFILES="pam.c"
OUTNAME=h0h0

all: $(OUTNAME).so

$(OUTNAME).so:
  $(CC) $(CFLAGS) -o $(OUTNAME).so $(CFILES)

clean:
	rm -f *.so
