# Execute Assembly PICO

A PICO for Crystal Palace that implements native CLR hosting to execute an appended .NET assembly in memory, using the same technique as the [donut](https://github.com/TheWover/donut) shellcode generator. This allows you to load .NET tools like Rubeus or Seatbelt from position-independent code without dropping them on the disk.

![A screenshot of the PICO in action, executing an appended Seatbelt assembly](img/execute-assembly-2.png)

## Building

You will need MinGW GCC, the zip utility, and a `crystal-palace` directory containing the CPL executables needed to link the project. Modify `config.spec` to specify the .NET assembly you want to link with the project and the commandline arguments you want to pass.

To build the PICO by itself (as a COFF), run `make pico`. To build the example runner and link it with the PICO, run `make runner`.

The example runner is written to `out/runner.bin`. The default configuration invokes an appended Rubeus assembly to execute "asktgt" with some generic dummy creds when you execute it:

![A screenshot of the PICO in action, executing an appended Rubeus assembly](img/execute-assembly-1.png)

## Using the PICO

The signature of the PICO's entrypoint is:

```c
HRESULT (*EXECUTE_ASSEMBLY_PICO)(char *assembly, size_t assembly_len, WCHAR *argv[], int argc);
```

The first two arguments should contain a pointer to a raw .NET assembly and its size. The second two arguments are used to pass string parameters to the assembly when it is invoked.

## Capturing Output

This PICO just invoke the assembly with the arguments you provided. It doesn't make any attempt to capture the output. 

If you need to provide input via STDIN or capture the output from STDOUT, you should modify your loader to use WIN32 API calls like `CreatePipe()` and `SetStdHandle()` and connnect the standard I/O devices for your process to an anonymous pipe. Then you can read from and write to it as usual.

## Credits

- [Raphael Mudge](https://tradecraftgarden.org/crystalpalace.html) for Crystal Palace and LibTCG.
- [XPNSec](https://gist.github.com/xpn/e95a62c6afcf06ede52568fcd8187cc2) for this gist which helped me understand the basic process of CLR hosting.
- [TheWover](https://github.com/TheWover/donut/blob/master/loader/inmem_dotnet.c) for the Donut generator/loader. I referenced its in-memory assembly execution extensively and used `clr.h` directly in the PICO.
