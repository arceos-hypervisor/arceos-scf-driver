OUTPUT_DIR := ../output
EXECUTABLE := $(OUTPUT_DIR)/shadow-process
LDSCRIPT := shadow-process.lds
OBJECTS := arceos_scf.o arceos_vdev.o main.o
CC := gcc
CFLAGS := -static -O3 -Wall -Werror -D__LINUX_COMPILER_TYPES_H

# WARNING: musl-gcc will place .data to 0x400000 even if we specify -Ttext-segment=0x200000
# DO NOT USE musl-gcc until this issue is resolved
$(EXECUTABLE): $(OBJECTS) $(OUTPUT_DIR)
	$(CC) -static $(OBJECTS) -o $(EXECUTABLE) -Wl,-Ttext-segment=0x200000

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

rebuild: clean $(EXECUTABLE)
