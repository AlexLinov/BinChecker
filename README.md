# Windows Service Permissions Checker

This project contains a C program designed to check the permissions of binaries associated with running Windows services, focusing specifically on identifying executables where `BUILTIN\Users` have `F` (Full Control) permissions. Other scripts such as PowerUp can provide this identification but I figured why not utilize native commands and not trigger AV. Tested on Windows 11 w/ current Defender.

Added functionality to search for directories with write access

![image](https://github.com/AlexLinov/BinChecker/assets/74632540/f8704467-056f-4535-b742-75ee1c9141b6)


## Overview

The Windows Service Permissions Checker scans running services on a Windows system, extracts the path to their executables, and uses the `icacls` command to inspect the permissions applied to these files. It's particularly useful for system administrators and security professionals looking to audit service executable permissions.

## Features

- Lists all running Windows services and their executable paths.
- Checks and prints paths where `BUILTIN\Users` have `F` permissions.
- Supports executables with paths that include spaces and are enclosed in quotes.

## Command-Line Arguments Added

### Quick Start
- **Find all .exe files**: Run without options.
  ```cmd
  example.exe
  ```
- **Find directories with write permission**: Use `-write`.
  ```cmd
  example.exe
  ```
- **Find the first .exe file only**: Use `-quick`.
  ```cmd
  example.exe -quick
  ```
- **Find the first .exe file only**: Use `-full`.
  ```cmd
  example.exe -full  
- **Help**: Use `-help` for usage information.
  ```cmd
  example.exe -help
  ```

### Summary
Now supporting command-line arguments, the tool lets users find `.exe` files where `BUILTIN\Users` have Full Control. Use `-quick` to stop after finding the first such file, speeding up the search.


## Prerequisites

- Windows Operating System
- GCC for Windows (MinGW or equivalent) to compile the source code
- PowerShell access for running the provided script

## Compilation

To compile the program, use the following mingw32 command:

```bash
x86_64-w64-mingw32-gcc input.c -o output.exe

> Pre-Compiled binary is included but feel free to do it yourself.
```
## To-Do
- Turn this into a more of an autopwn.
- After identifying service binaries with Full Control, attempt to copy that binary to current directory and replace with simple net user add executable
- Thoughts?!?!?!?!
