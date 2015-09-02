
editor: editor.c
	gcc -oeditor -DSCALE=1 -DFONTEDITOR editor.c fontdata.cpp -lSDL -lm -O3 -ffast-math -march=k8 -Wall
	
editor.exe: editor.c
	i686-w64-mingw32-gcc -oeditor.exe -DSCALE=1 -DFONTEDITOR editor.c fontdata.cpp -lmingw32 -lm -lSDLmain -lSDL -O3 -ffast-math -march=k8 -Wall -mwindows
	
packer: packer.c
	gcc -opacker -DFONTEDITOR packer.c fontdata.cpp -lm -O3 -ffast-math -march=k8 -Wall
	
packer.exe: packer.c
	i686-w64-mingw32-gcc -opacker.exe -DFONTEDITOR packer.c fontdata.cpp -lm -O3 -ffast-math -march=k8 -Wall
	
unpacker: unpacker.c
	gcc -ounpacker -DFONTEDITOR unpacker.c fontdata.cpp -lm -O3 -ffast-math -march=k8 -Wall

unpacker.exe: unpacker.c
	i686-w64-mingw32-gcc -ounpacker.exe -DFONTEDITOR unpacker.c fontdata.cpp -lm -O3 -ffast-math -march=k8 -Wall
	
clean:
	rm -f editor packer unpacker editor.exe packer.exe unpacker.exe
