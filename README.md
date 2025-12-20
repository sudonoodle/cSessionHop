# cSessionHop

**cSessionHop** is a Beacon Object File (BOF) for Cobalt Strike, designed to hijack user sessions through the `IHxHelpPaneServer` COM interface. This technique allows operators to execute processes in other user sessions without relying on traditional methods like process injection or credential dumping.

This BOF interacts with the `IHxHelpPaneServer` COM interface (CLSID: `8cec58ae-07a1-11d9-b15e-000d56bfe6ee`), which runs with the privileges of the interactive user in the target session. By binding to this COM object across session boundaries using the session moniker (`session:<id>!new:<clsid>`), it can execute processes within the context of the specified user's session.


## Usage

To use cSessionHop, load the BOF and Aggressor script in Cobalt Strike. Then, execute the desired process in a target session with the following command:

```
beacon> cSessionHop <session_id> <executable>
```

### Examples

```
# Execute notepad in session 1
beacon> cSessionHop 1 notepad.exe

# Execute cmd.exe in session 2
beacon> cSessionHop 2 cmd.exe

# Execute a specific binary with full path
beacon> cSessionHop 1 C:\Windows\System32\cmd.exe
```

## Building

This project uses MinGW for compilation. To build the BOF, simply run the following command:

```bash
make
```

This will compile both x86 and x64 versions of the BOF from the `src` directory.

### Build Requirements

* **MinGW-w64** (x86_64-w64-mingw32-gcc and i686-w64-mingw32-gcc)
* **Make**

## Credits

This project builds on prior research and implementations:

- **James Forshaw**: Original research on session moniker abuse
  - [Raising The Dead: Abusing Elevation on Windows](https://googleprojectzero.blogspot.com/2016/01/raising-dead.html)

- **Michael Zhmailo (CICADA8)**: Documentation on the IHxHelpPaneServer technique
  - [Process Injection is Dead, Long Live IHxHelpPaneServer](https://cicada-8.medium.com/process-injection-is-dead-long-live-ihxhelppaneserver-af8f20431b5d)

- **Andrew Oliveau (3lp4tr0n)**: Original C# SessionHop implementation
  - [SessionHop GitHub Repository](https://github.com/3lp4tr0n/SessionHop)
