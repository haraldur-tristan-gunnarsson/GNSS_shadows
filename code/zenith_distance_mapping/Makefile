CC = gcc
WARNINGS =  -Wall -Wextra -pedantic -Wstrict-prototypes -Wpointer-arith -Wcast-align \
			-Wwrite-strings \
			-Wredundant-decls -Wnested-externs -Winline \
			-Wuninitialized# -Wconversion -Wmissing-declarations -Wshadow -Wmissing-prototypes
CFLAGS = --std=c11 $(WARNINGS) -Werror# -O2 -Og #-O0
GLFW_PATH = ../../3rd_party_code/glfw-2.7.9
GLFW_SO_PATH = $(GLFW_PATH)/lib/x11
LIBS = -lpng -lm -I$(GLFW_PATH)/include -L$(GLFW_SO_PATH) -Wl,-rpath=$(GLFW_SO_PATH) \
	   -lglfw -lGLEW -lGL# -lassimp

all: zen_dist_map.elf shadow_mapped.elf height_map.elf shad_vol_zdmaps.elf

zen_dist_map.elf: main.c render_zen_dist_map.c chores.c
	cd $(GLFW_PATH) && $(MAKE) -s -f Makefile x11
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

shad_vol_zdmaps.elf: main.c render_shad_vol_zdmaps.c chores.c
	cd $(GLFW_PATH) && $(MAKE) -s -f Makefile x11
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

shadow_mapped.elf: main.c render_shadow_mapped.c chores.c
	cd $(GLFW_PATH) && $(MAKE) -s -f Makefile x11
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

height_map.elf: main.c render_height_map.c chores.c
	cd $(GLFW_PATH) && $(MAKE) -s -f Makefile x11
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)


#.c.o:
#	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f *.elf
	cd $(GLFW_PATH) && $(MAKE) -f Makefile x11-dist-clean
#	-rm -f *.o

rebuild: clean all

.PHONY: all clean rebuild
