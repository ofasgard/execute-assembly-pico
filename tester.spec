x64:
	# Load the runner PIC.
	load "bin/runner.o"
	make pic +optimize +gofirst
	
	# Merge in libraries.
	mergelib "lib/libtcg/libtcg.x64.zip"
	
	# Opt into dynamic function resolution using the resolve() function.
	dfr "resolve" "ror13"
	
	# Load the PICO
	load "bin/execute_assembly.test.o"
	make object +optimize
	
	# Merge in libraries.
	mergelib "lib/libtcg/libtcg.x64.zip"
	mergelib "lib/libcpltest/libcpltest.x64.zip"
	
	# Export as bytes and link as "my_pico".
	export
	link "my_pico"
	
	# Load the .NET assembly and link as "my_assembly".
	load "bin/Rubeus.exe"
	preplen
	link "my_assembly"
	
	# Export the resulting PIC.
	export
