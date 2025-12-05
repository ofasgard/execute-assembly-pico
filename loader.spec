x64:
	# Load the runner PIC.
	load "bin/runner.o"
	make pic +optimize +gofirst
	
	# Merge in LibTCG.
	mergelib "lib/libtcg/libtcg.x64.zip"
	
	# Opt into dynamic function resolution using the resolve() function.
	dfr "resolve" "ror13"
	
	# Load the PICO
	load "bin/execute_assembly.o"
	make object +optimize
	
	# Export as bytes and link as "my_pico".
	export
	link "my_pico"
	
	# Load the .NET assembly and link as "my_assembly".
	resolve "%ASSEMBLY_PATH"
	load %ASSEMBLY_PATH
	preplen
	link "my_assembly"
	
	# Export the resulting PIC.
	export
