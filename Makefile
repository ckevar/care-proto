HEADERS := $(wildcard include/*.h)
SRC := $(wildcard src/*.c)
SYNC := $(SRC:%.c=%.sync)
SYNH := $(HEADERS:%.h=%.synh)
DEST_DIR = /home/pi/code/care/
syn: $(SYNC) $(SYNH)

%.sync: %.c
	@touch $@
	@sshpass -p 'raspberry' scp $^ pi@192.168.43.100:$(DEST_DIR)$^
	@echo "copying $^"

%.synh: %.h
	@touch $@
	@sshpass -p 'raspberry' scp $^ pi@192.168.43.100:$(DEST_DIR)$^
	@echo "copying $^"

synca:
	scp src/*.c pi@192.168.43.100:$(DEST_DIR)src/.
