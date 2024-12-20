<!--
SPDX-FileCopyrightText: 2024 Hanabusa Masahiro
SPDX-License-Identifier: CC-BY-4.0
-->

# C/C++/Rust toolchain for Windows

In Linux, GCC is included in the OS as a system compiler, but in Windows, it is necessary to install a compiler separately from the OS.

If software development itself is your business, you should not be stingy and purchase Visual Studio Pro or subscribe to MSDN subscription. 
However, it would be a waste to subscribe to MSDN if you just want to write some code to compile the results of an experiment or create a simple business improvement tool for internal use. 
This is a tip for such people.

## MSVC license issue

MSVC (Microsoft Visual C++) is a genuine Microsoft compiler, but the conditions under which it can be used are complicated. The following is an overview as of Visual Studio 2022. For the exact conditions of use, please check the [license term](https://visualstudio.microsoft.com/en/license-terms/) by yourself.

- If you use Visual Studio 2017 Express, there are few restrictions. However, Visual Studio 2017 is the final version, and it is no longer provided.
- For purely personal use, Visual Studio 2022 Community can be used.
- If you use Visual Studio 2022 Community in an organization, there are restrictions such as a limit of 5 users, and it is difficult to use it in the workplace with proper license management.
  - It is also difficult to apply the OSS exception clause properly.

## Visual Studio 2017 Express for Windows Desktop

If you don't want to sign an MSDN contract, one option is to use Express, which [has fewer license restrictions](https://visualstudio.microsoft.com/en/license-terms/mlt080317/). 
Download and install [Visual Studio Express 2017 for Desktop installer](https://aka.ms/vs/15/release/vs_WDExpress.exe) and [Build Tools installer](https://aka.ms/vs/15/release/vs_buildtools.exe). Download and install Visual Studio Express 2017 for Desktop installer and Build Tools installer.

If you want to use Rust together, you can install the `x86_64-pc-windows-msvc` version of the Rust compiler. (rustup-init will install this automatically)

## llvm-mingw toolchain

As an alternative to MSVC, we recommend the [LLVM MinGW toolchain](https://github.com/mstorsjo/llvm-mingw).

This toolchain uses the libraries included in MinGW, the Windows version of GCC, and replaces the MinGW compiler with LLVM Clang. 
It has advantages over MSVC for cross-platform development.

- Compared to MSVC
  - Free & open source, so it can be used freely.
  - Clang is also available for Linux and Mac, so you can unify compilers if you plan to write code for multiple platforms.
  - It is easy to cross-compile Windows target on a Linux host. This is especially useful for building CI environments.
- Compared to GCC MinGW
  - LLVM MinGW uses ucrt (Universal C Runtime) which is standard since Windows 10 as C runtime, so there is no problem even if combined with DLLs built with recent MSVC.
  - Features not available in MinGW GCC, such as thread local storage, are also available.
  - The aarch64-pc-windows target, which is not supported by GCC, can also be used.

### How to use LLVM MinGW

Just extract the ZIP file and set the PATH environment variable.

- (1) Get `llvm-mingw-*-ucrt-x86_64.zip` (AMD, Intel PC) or `llvm-mingw-*-ucrt-aarch64.zip` (ARM64 PC) from the [LLVM MinGW release page](https://github.com/mstorsjo/llvm-mingw/releases).
- (2) Extract the contents of the ZIP (any location is fine)
  - Assuming you have `C:\opt\llvm-mingw\bin\x86_64-w64-mingw32-clang.exe` as this example)
- (3) Set environment variables `PATH=C:\opt\llvm-mingw\bin;<other PATHs>`

Now you can compile from console as follows.

``` console
C:\somewhere> x86_64-w64-mingw32-clang.exe -o hello.exe hello.c
C:\somewhere> hello.exe
Hello, World!
```

Build tools such as `MSBuild` included in MSVC are not included in LLVM MinGW, so it is better to use them in combination with [CMake](https://cmake.org/) and others.

## Rust \*-pc-windows-gnullvm target

Rust [\*-pc-windows-gnullvm target has been tier 2](https://github.com/rust-lang/rust/pull/121712) since [1.79.0](https://github.com/rust-lang/rust/releases/tag/1.79.0) and can [be easily used with LLVM MinGW](https://doc.rust-lang.org/rustc/platform-support/pc-windows-gnullvm.html).

###  Using Rust + llvm-mingw on Windows host

The procedure is the following 3 steps.

The major version of LLVM used internally by the Rust compiler and the major version of LLVM MinGW must match (can be checked at `rustc -V -v` ). Rust 1.73-1.77 =\> LLVM 17, Rust 1.78-1.81 =\> LLVM 18 , Rust 1.82- =\> LLVM 19

- (1) Get Rust compiler from `x86_64-pc-windows-gnu` with rustup  
    ``` console
    C:\somewhere> rustup-init.exe --default-host x86_64-pc-windows-gnu --target x86_64-pc-windows-gnullvm
    ...
    ```
  - `rustup-init` explicitly specify `x86_64-pc-windows-gnu` as the host toolchain. If you do not specify it, `x86_64-pc-windows-msvc` will be retrieved.
  - `--target x86_64-pc-windows-gnullvm` (1) It is convenient to retrieve  at the same time.
-  (2) Duplicate `libgcc*.a` to llvm-mingw's library path
  - `C:\Users\<username>\.rustup\toolchains\stable-x86_64-pc-windows-gnu\lib\rustlib\x86_64-pc-windows-gnu\lib\self-contained` Duplicate `libgcc.a`, `libgcc_eh.a`, `libgcc_s.a` in `C:\opt\llvm-mingw\x86_64-w64-mingw32\lib` 
  - This is a hack for Rust compiler of `x86_64-pc-windows-gnu` to link libgcc for GCC. It is necessary for compile of `build.rs`.
- (3: Optional) Set default target specification in `.cargo/config.toml`   
    ``` toml
    [build]
    target = "x86_64-pc-windows-gnullvm"
    ```
    `.cargo/config.toml` If you don't write the setting in \[build\], specify target at build time and use it.
    ``` console
    C:\somewhere> cargo build --target x86_64-pc-windows-gnullvm
    ```

`x86_64-pc-windows-gnu` The build will be cross compiled to another `x86_64-pc-windows-gnullvm` target on the host of As mentioned above, `build.rs` is compiled by host compiler (-gnu), but the final .exe is compiled by target (-gnullvm).

### Using the Windows .msi installer

I think there are few people who install Rust with .msi, in that case, do as follows.

- (1) Obtain [rust-1.83.0-x86\_64-pc-windows-gnu.msi](https://static.rust-lang.org/dist/rust-1.83.0-x86_64-pc-windows-gnu.msi) from [Other Rust Installation Methods](https://forge.rust-lang.org/infra/other-installation-methods.html) and install
- (2) Obtain and extract [rust-std-1.83.0-x86\_64-pc-windows-gnullvm.tar.xz](https://static.rust-lang.org/dist/2024-11-28/rust-std-1.83.0-x86_64-pc-windows-gnullvm.tar.xz) 
  - Put `x86_64-pc-windows-gnullvm` in `rust-std-x86_64-pc-windows-gnullvm/lib/rustlib` of .tar.gz into `C:\Program Files\Rust GNU 1.83\lib\rustlib` 

### Cross compile on Linux host

To cross compile .exe for Windows target on a Linux host, do the following

- (1) Get `llvm-mingw-*-ucrt-ubuntu-*-x86_64.tar.xz` from the [llvm-mingw release page](https://github.com/mstorsjo/llvm-mingw/releases).
- (2) Extract the contents of the ZIP file (any location is fine)
  - The following is an example of the state in which `/opt/llvm-mingw/bin/x86_64-w64-mingw32-clang` is located.
- (3) Set environment variables `PATH=/opt/llvm-mingw/bin:<other PATHs>`
- (4) Install `x86_64-unknown-linux-gnu` host toolchain (normal Rust install procedure)  
     ``` console
     $ curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
    ```
- (5) Add `x86_64-pc-windows-gnullvm` with rustup  
    ``` console
     $ rustup target add x86_64-pc-windows-gnullvm
    ```

Rust cross compile as follows

``` console
$ cargo build --target x86_64-pc-windows-gnullvm
```

This will generate a Windows .exe.

