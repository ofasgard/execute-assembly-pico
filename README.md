# Execute Assembly PICO

A PICO for Crystal Palace that implements native CLR hosting to execute an appended .NET assembly in memory, using the same technique as the [donut](https://github.com/TheWover/donut) shellcode generator. This allows you to load .NET tools like Rubeus or Seatbelt from position-independent code without dropping them on the disk.

![A screenshot of the PICO in action, executing an appended Seatbelt assembly](img/execute-assembly-2.png)

## Building

You will need MinGW GCC, the zip utility, and a `crystal-palace` directory containing the CPL executables needed to link the project. Note that the example Makefile expects to find a Rubeus assembly at `bin/Rubeus.exe`, which you will have to provide yourself.

To build the PICO by itself, run `make pico`. To build the example runner and link it with the PICO, run `make runner`.

The example runner is written to `out/runner.bin`, and it invokes the appended Rubeus assembly to execute "asktgt" with some generic dummy creds when you execute it:

![A screenshot of the PICO in action, executing an appended Rubeus assembly](img/execute-assembly-1.png)

## Using the PICO

The signature of the PICO's entrypoint is:

```c
HRESULT (*EXECUTE_ASSEMBLY_PICO)(char *assembly, size_t assembly_len, WCHAR *argv[], int argc);
```

The first two arguments should contain a pointer to a raw .NET assembly and its size. The second two arguments are used to pass string parameters to the assembly when it is invoked.

## Configuring the Runner

The example runner is mostly intended to demonstrate the functionality of the PICO, but you can tweak it to invoke other assemblies if desired. To do so:

- Adjust `Makefile` with the correct path to your desired .NET assembly, i.e. `'%ASSEMBLY_PATH=bin/SomeAssembly.exe'`.
- Adjust `runner.c` to change the hardcoded arguments passed to the PICO by the `go()` function.

Then run `make runner` again to build your PIC shellcode.

## Credits

- [Raphael Mudge](https://tradecraftgarden.org/crystalpalace.html) for Crystal Palace and LibTCG.
- [XPNSec](https://gist.github.com/xpn/e95a62c6afcf06ede52568fcd8187cc2) for this gist which helped me understand the basic process of CLR hosting.
- [TheWover](https://github.com/TheWover/donut/blob/master/loader/inmem_dotnet.c) for the Donut generator/loader. I referenced its in-memory assembly execution extensively and used `clr.h` directly in the PICO.
