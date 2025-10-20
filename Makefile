dirs:
	mkdir -p bin out

libtcg: lib/libtcg/
	cd lib/libtcg && make all
	
libcpltest: lib/libcpltest/
	cd lib/libcpltest && make all
	
pico: dirs src/execute_assembly.c src/headers/
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/execute_assembly.c -o bin/execute_assembly.o
	
runner: dirs libtcg pico crystal-palace/ loader.spec src/runner.c
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./loader.spec x64 out/runner.bin
	
tester: dirs libtcg libcpltest runner crystal-palace/ loader.spec src/runner.c
	x86_64-w64-mingw32-gcc -DCPLTESTS -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/execute_assembly.c -o bin/execute_assembly.test.o
	java -Dcrystalpalace.verbose=false -classpath crystal-palace/crystalpalace.jar crystalpalace.spec.LinkSpec buildPic ./tester.spec x64 out/tester.bin
	
clean: bin out lib/libtcg/ lib/libcpltest/
	rm -f bin/*.o
	rm -f out/*.bin
	cd lib/libtcg && make clean
	cd lib/libcpltest && make clean
