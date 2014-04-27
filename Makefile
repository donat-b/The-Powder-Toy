SOURCES := $(wildcard src/*.cpp src/*/*.cpp src/*/*/*.cpp src/socket/*.c)
HEADERS := $(wildcard includes/*.h src/*/*.h src/simulation/elements/*.h src/socket/*.h)
OBJS := $(patsubst src/%.c,build/obj/%.o,$(patsubst src/%.cpp,build/obj/%.o,$(SOURCES)))

CFLAGS := -w -Iincludes/ -Isrc/ -D_GNU_SOURCE
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -fomit-frame-pointer -funsafe-loop-optimizations -Wunsafe-loop-optimizations
LFLAGS := -lpthread -lSDL -lfftw3f -lm -lbz2 -lX11 -llua5.1 -lrt
LFLAGS_X := -lm -lbz2 -lSDLmain
LFLAGS_WIN := -lmingw32 -lgnurx -lws2_32 -lSDLmain -lpthread -lSDL -lfftw3f -lm -lbz2 -llua5.1
LFLAGS_WINCROSSCOMPILE := -lmingw32 -Wl,-Bstatic -lfftw3f -lgnurx -lSDLmain -lSDL -lpthread -lm -lbz2 -llua5.1 -Wl,-Bdynamic -lws2_32 -lwinmm -ldxguid
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -pg -g
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

# windows c
#COMPILER := gcc
#CC := gcc
#CC_WIN := gcc.exe
#WIN_RES := windres.exe

# windows c++
#CC := g++
#CC_WIN := g++.exe
#WIN_RES := windres.exe

#others (Linux) c. Change the CC_WIN and WIN_RES to match your mingw installation
CC := gcc -std=c99 -D_POSIX_C_SOURCE=200112L
CXX := g++ -std=c++98
CC_WIN := i586-mingw32msvc-gcc -std=gnu99 -D_POSIX_C_SOURCE=200112L
CXX_WIN := i586-mingw32msvc-g++ -std=gnu++98
WIN_RES := i586-mingw32msvc-windres

#32bit linux
powder: build/powder
powder-sse: build/powder-sse
powder-sse2: build/powder-sse2
powder-sse3: build/powder-sse3
powder-debug: build/powder-debug
#64bit linux
powder-64: build/powder-64
powder-64-sse2: build/powder-64-sse2
powder-64-sse3: build/powder-64-sse3
powder-64-debug: build/powder-64-debug
#32bit windows
powder-sse.exe: build/powder-sse.exe
powder-sse2.exe: build/powder-sse2.exe
powder-sse3.exe: build/powder-sse3.exe
#cross compiling to windows from linux
powder-crosscompile.exe: build/powdercrosscompile.exe
powder-crosscompile-sse2.exe: build/powdercrosscompile-sse2.exe
#opengl
powder-sse3-opengl: build/powder-sse3-opengl
powder-sse3-opengl: build/powder-sse3-opengl

# general compiler flags
build/powder build/powder-sse build/powder-sse2 build/powder-sse3 build/powder-sse3-opengl: CFLAGS += -m32 -DLIN32 $(OFLAGS)
build/powder-debug: CFLAGS += -m32 -DLIN32 $(FLAGS_DBUG)
build/powder-64: CFLAGS += -DINTERNAL -DLIN64 $(OFLAGS)
build/powder-64-sse2 build/powder-64-sse3 build/powder-64-sse3-opengl: CFLAGS += -m64 -DLIN64 $(OFLAGS)
build/powder-64-debug: CFLAGS += -m64 -DLIN64 $(FLAGS_DBUG)
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: CFLAGS += -mwindows -DWIN32 $(OFLAGS)
build/powder-sse3-opengl build/powder-64-sse3-opengl: CFLAGS += -DOGLR -DPIX32OGL -DPIXALPHA
build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: CFLAGS += -DPTW32_STATIC_LIB -D_WIN32_WINNT=0x0500

# SSE flags:
build/powder-sse3 build/powder-sse3-opengl build/powder-64-sse3 build/powder-64-sse3-opengl build/powder-sse3.exe: CFLAGS += -march=native -DX86 -DX86_SSE3 -msse3
build/powder-sse2 build/powder-64-sse2 build/powder-sse2.exe build/powdercrosscompile-sse2.exe: CFLAGS += -march=native -DX86 -DX86_SSE2 -msse2
build/powder-sse build/powder-sse.exe: CFLAGS += -march=native -DX86 -DX86_SSE
build/powder build/powder-64 build/powder-debug build/powder-64-debug build/powdercrosscompile.exe: CFLAGS += -march=native -DX86

# libs:
build/powder build/powder-sse build/powder-sse2 build/powder-sse3 build/powder-debug build/powder-sse3-opengl build/powder-64 build/powder-64-sse2 build/powder-64-sse3 build/powder-64-debug build/powder-64-sse3-opengl: LIBS += $(LFLAGS)
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe: LIBS += $(LFLAGS_WIN)
build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: LIBS += $(LFLAGS_WINCROSSCOMPILE)
build/powder-64-sse3-opengl build/powder-sse3-opengl: LIBS += -lGL

# extra windows stuff
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: EXTRA_OBJS += build/obj/powder-res.o
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: CC := $(CC_WIN)
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: CXX := $(CXX_WIN)
build/powder-sse.exe build/powder-sse2.exe build/powder-sse3.exe build/powdercrosscompile.exe build/powdercrosscompile-sse2.exe: build/obj/powder-res.o


#Create different .o files for each build <filename>.<buildname>.o
build/powder: $(patsubst build/obj/%.o,build/obj/%.powder.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-sse: $(patsubst build/obj/%.o,build/obj/%.powder-sse.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse.o: src/%.cpp $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse2: $(patsubst build/obj/%.o,build/obj/%.powder-sse2.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse2.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse2.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse2.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-sse3: $(patsubst build/obj/%.o,build/obj/%.powder-sse3.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse3.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse3.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-debug: $(patsubst build/obj/%.o,build/obj/%.powder-debug.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-debug.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder-debug.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-debug.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-sse3-opengl: $(patsubst build/obj/%.o,build/obj/%.powder-sse3-opengl.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3-opengl.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse3-opengl.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse3-opengl.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<


build/powder-64: $(patsubst build/obj/%.o,build/obj/%.powder-64.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-64.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-64-sse2: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse2.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse2.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse2.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-64-sse2.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-64-sse3: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse3.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-64-sse3.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-64-debug: $(patsubst build/obj/%.o,build/obj/%.powder-64-debug.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-debug.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder-64-debug.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-64-debug.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

build/powder-64-sse3-opengl: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3-opengl.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3-opengl.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse3-opengl.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-64-sse3-opengl.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<

# On Windows builds, an extra compiler flag is needed to fix stack alignment
# When Windows creates the gravity calculation thread, it has 4 byte stack alignment
# But we need 16 byte alignment so that SSE instructions in FFTW work without crashing
build/powder-sse.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse.exe.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse.exe.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<
build/obj/gravity.powder-sse.exe.o: src/gravity.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -mstackrealign -o $@ $<

build/powder-sse2.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse2.exe.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse2.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse2.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse2.exe.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<
build/obj/gravity.powder-sse2.exe.o: src/gravity.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -mstackrealign -o $@ $<

build/powder-sse3.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse3.exe.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse3.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powder-sse3.exe.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<
build/obj/gravity.powder-sse3.exe.o: src/gravity.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -mstackrealign -o $@ $<

build/powdercrosscompile.exe: $(patsubst build/obj/%.o,build/obj/%.powdercrosscompile.exe.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powdercrosscompile.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powdercrosscompile.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powdercrosscompile.exe.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<
build/obj/gravity.powdercrosscompile.exe.o: src/gravity.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -mstackrealign -o $@ $<

build/powdercrosscompile-sse2.exe: $(patsubst build/obj/%.o,build/obj/%.powdercrosscompile-sse2.exe.o,$(OBJS))
	$(CXX) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powdercrosscompile-sse2.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powdercrosscompile-sse2.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<
build/obj/%.powdercrosscompile-sse2.exe.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -o $@ $<
build/obj/gravity.powdercrosscompile-sse2.exe.o: src/gravity.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) -mstackrealign -o $@ $<





.PHONY: clean

clean:
	rm -f build/obj/*.o
	rm -f build/obj/*/*.o
	rm -f build/obj/*/*/*.o



powder-icc: $(SOURCES)
	/opt/intel/Compiler/11.1/073/bin/intel64/icc -m64 -o$@ -Iincludes/ -O2 -march=core2 -msse3 -mfpmath=sse -lSDL -lbz2 -lm -xW $(SOURCES) -std=c99 -D_POSIX_C_SOURCE=200112L

build/obj/powder-res.o: src/Resources/powder-res.rc src/Resources/powder.ico src/Resources/document.ico
	cd src/Resources && $(WIN_RES) powder-res.rc powder-res.o
	mv src/Resources/powder-res.o build/obj/powder-res.o

powder-x: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL
	strip $@ 
	mv $@ build
powder-x-ogl: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework OpenGL
	strip $@ 
	mv $@ build
