CC=gcc
CFLAGS="-Wall" "-fPIC" "-shared" "-ldl"
OUTNAME=h0h0

all: $(OUTNAME).so

$(OUTNAME).so:
	@echo "[+] Compiling: $(OUTNAME).c -> $(OUTNAME).so"
	$(CC) $(CFLAGS) -o $(OUTNAME).so $(OUTNAME).c
	@echo

clean:
	@echo '[+] Cleaning up...'
	rm -f *.so
