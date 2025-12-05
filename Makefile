dirs:
	mkdir -p bin out

libtcg: lib/libtcg/
	cd lib/libtcg && make all
	
libcpltest: lib/LibCPLTest/
	cd lib/LibCPLTest && make all
	
pico: dirs src/execute_assembly.c src/headers/
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/execute_assembly.c -o bin/execute_assembly.o
	
runner: dirs libtcg pico crystal-palace/ loader.spec src/runner.c
	x86_64-w64-mingw32-gcc -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/runner.c -o bin/runner.o
	java -Dcrystalpalace.verbose=false -jar crystal-palace/crystalpalace.jar buildPic ./loader.spec x64 out/runner.bin '%ASSEMBLY_PATH=bin/Rubeus.exe'
	
tester: dirs libtcg libcpltest runner crystal-palace/ loader.spec src/runner.c
	x86_64-w64-mingw32-gcc -DCPLTESTS -DWIN_X64 -shared -masm=intel -Wall -Wno-pointer-arith -fno-toplevel-reorder -c src/execute_assembly.c -o bin/execute_assembly.test.o
	java -Dcrystalpalace.verbose=false -jar crystal-palace/crystalpalace.jar buildPic ./tester.spec x64 out/tester.bin '%ASSEMBLY_PATH=bin/Rubeus.exe'
	
clean: bin out lib/libtcg/ lib/LibCPLTest/
	rm -f bin/*.o
	rm -f out/*.bin
	cd lib/libtcg && make clean
	cd lib/LibCPLTest && make clean
