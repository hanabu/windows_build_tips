<!--
SPDX-FileCopyrightText: 2024 Hanabusa Masahiro
SPDX-License-Identifier: CC-BY-4.0
-->
# Windows 向けの C/C++/Rust toolchain

Linux では OS に GCC が system compiler として付属しているが、Windows では OS とは別途 compiler を導入する必要がある。

Software 開発自体が業務であれば、ケチらずに Visual Studio Pro 購入もしくは MSDN subscription を契約というのが本筋だろう。
しかし、実験結果を集計するためにちょっと code 書くとか、社内利用の簡単な業務改善 tool を作るといった用途で MSDN 契約をするのはちょっともったいない。
これはそんな人のための tips。

## MSVC の license issue

Microsoft の純正 compiler として、MSVC (Microsoft Visual C++) があるが、利用できる条件がややこしい。以下は Visual Studio 2022 時点の概要。厳密な利用条件は各自 [license term](https://visualstudio.microsoft.com/ja/license-terms/) を確認のこと。

- Visual Studio 2017 Express であれば利用制限は少ない。Express license で BuildTool も利用許諾されている。しかし、Visual Studio 2017 が最終版で、それ以降の提供はなくなってしまった。
- 純粋に個人利用であれば Visual Studio 2022 Community を利用可能。
- 組織で Visual Studio 2022 Community 使う場合は 5名までの制限などあり、正しく license 管理された状態で職場で利用するのは難しい。
  - Open source 例外条項もあるが、OSS として公開するほどでもない小物 tool も書くことは十分想定される。OSS 例外条項をきちんと適用することも意外と難しい。 

## Visual Studio 2017 Express for Windows Desktop

MSDN 契約をするほどでもない場合の選択として、[license 上の利用制限が少ない](https://visualstudio.microsoft.com/ja/license-terms/mlt080317/)Express 使うのが1つの方法。
[Visual Studio Express 2017 for Desktop installer](https://aka.ms/vs/15/release/vs_WDExpress.exe) と [Build Tools installer](https://aka.ms/vs/15/release/vs_buildtools.exe) を download して install する。

Rust を一緒に使う場合は、`x86_64-pc-windows-msvc` 版の Rust compiler を入れればよい。(rustup-init はこちらを自動で入れてくれる)

## llvm-mingw toolchain

MSVC 以外のお薦めとして、[LLVM MinGW toolchain](https://github.com/mstorsjo/llvm-mingw) がある。

これは GCC の Windows 版である MinGW に含まれる library 類を利用し、MinGW の compiler を GCC から LLVM Clang に置き換えたもの。
Cross platform での開発を前提とすると MSVC よりも利点がある。

- MSVC と比較して:
  - Free & open source なので、自由に利用可能。
  - Linux, Mac でも Clang は利用できるので、複数の platform 向けの code を書くつもりなら compiler を統一できる。
  - Linux host で Windows target の cross compile を簡単にできる。特に CI 環境構築に便利。
- GCC MinGW と比較して:
  - LLVM MinGW は C runtime として Windows 10 以降標準の ucrt (Universal C Runtime) を利用するため、最近の MSVC で build された DLL と結合しても問題が出ない。
  - Thread local storage といった MinGW GCC で利用できない機能も利用可能。
  - GCC が support していない aarch64-pc-windows target も利用可能。

### LLVM MinGW の使い方

ZIP を展開して、 PATH 環境変数を設定するだけで利用可能。

- (1) [LLVM MinGW の release page](https://github.com/mstorsjo/llvm-mingw/releases) から、`llvm-mingw-*-ucrt-x86_64.zip` (AMD, Intel PC) か `llvm-mingw-*-ucrt-aarch64.zip` (ARM64 PC) を取得
- (2) ZIPの内容を展開 (任意の場所でよい)
  - `C:\opt\llvm-mingw\bin\x86_64-w64-mingw32-clang.exe` がある状態を例とする
- (3) 環境変数設定 `PATH=C:\opt\llvm-mingw\bin;<other PATHs>`

以上で利用可能になる。Console から以下のように compile できる。

```console
C:\somewhere> x86_64-w64-mingw32-clang.exe -o hello.exe hello.c
C:\somewhere> .\hello.exe
Hello, world!
```

MSVC に含まれる `MSBuild` のような build tool は LLVM MinGW には含まれないので、[CMake](https://cmake.org/) などと組み合わせて使うのがよい。

## Rust \*-pc-windows-gnullvm target

Rust も [1.79.0](https://github.com/rust-lang/rust/releases/tag/1.79.0) から [\*-pc-windows-gnullvm target が tier 2 になり](https://github.com/rust-lang/rust/pull/121712)、[簡単に LLVM MinGW と組み合わせて利用できる](https://doc.rust-lang.org/rustc/platform-support/pc-windows-gnullvm.html)ようになっている。

### Windows host で Rust + llvm-mingw を利用

手順としては、以下の 3 step で利用可能。

Rust compiler が内部で使っている LLVM の major version と LLVM MinGW の major version を合わせておく必要がある(`rustc -V -v` で確認可能)。Rust 1.73-1.77 => LLVM 17, Rust 1.78-1.81 => LLVM 18, Rust 1.82- => LLVM19 

- (1) `x86_64-pc-windows-gnu` の Rust compiler を rustup で取得
    ```console
    C:\somewhere> rustup-init.exe --default-host x86_64-pc-windows-gnu --target x86_64-pc-windows-gnullvm
    ...
    ```
  - `rustup-init` の際に、host toolchain として `x86_64-pc-windows-gnu` を明示的に指定する。指定しないと `x86_64-pc-windows-msvc` の方を取得してしまう。
  - `--target x86_64-pc-windows-gnullvm` もまとめて取得しておくと便利
- (2) `libgcc*.a` を llvm-mingw の library path に複製
  - `C:\Users\<username>\.rustup\toolchains\stable-x86_64-pc-windows-gnu\lib\rustlib\x86_64-pc-windows-gnu\lib\self-contained` にある `libgcc.a`, `libgcc_eh.a`, `libgcc_s.a` の3つを `C:\opt\llvm-mingw\x86_64-w64-mingw32\lib` に複製する
  - これは `x86_64-pc-windows-gnu` の Rust compiler が GCC 用の libgcc を link しようとするための hack。`build.rs` の compile に必要。
- (3: 任意) `.cargo/config.toml` で default target 指定を設定する  
    ```toml
    [build]
    target = "x86_64-pc-windows-gnullvm"
    ```
  `.cargo/config.toml` に設定を書かない場合は、build 時に target を指定して利用する。  
    ```console
    C:\somewhere> cargo build --target x86_64-pc-windows-gnullvm
    ```

`x86_64-pc-windows-gnu` の host 上で、別の `x86_64-pc-windows-gnullvm` target 向けに cross compile している状態になる。上記で触れているように、`build.rs` は host compiler (-gnu) で compile されるが、最終の .exe は target (-gnullvm) で compile される。

### Windows .msi installer を使う場合

.msi で Rust を install する人は少ないと思うが、その場合は以下のようにする。

- (1) [Other Rust Installation Methods](https://forge.rust-lang.org/infra/other-installation-methods.html) から [rust-1.83.0-x86_64-pc-windows-gnu.msi](https://static.rust-lang.org/dist/rust-1.83.0-x86_64-pc-windows-gnu.msi) を取得、install
- (2) [rust-std-1.83.0-x86_64-pc-windows-gnullvm.tar.xz](https://static.rust-lang.org/dist/2024-11-28/rust-std-1.83.0-x86_64-pc-windows-gnullvm.tar.xz) を取得して展開
  - .tar.gz の `rust-std-x86_64-pc-windows-gnullvm/lib/rustlib` にある `x86_64-pc-windows-gnullvm` を `C:\Program Files\Rust GNU 1.83\lib\rustlib` にいれる。

### Linux host で cross compile

Linux host で Windows target 用の .exe を cross compile する場合は、以下のようにする。

- (1) [llvm-mingw の release page](https://github.com/mstorsjo/llvm-mingw/releases) から、`llvm-mingw-*-ucrt-ubuntu-*-x86_64.tar.xz` を取得
- (2) ZIPの内容を展開 (任意の場所でよい)
  - `/opt/llvm-mingw/bin/x86_64-w64-mingw32-clang` がある状態を例とする
- (3) 環境変数設定 `PATH=/opt/llvm-mingw/bin:<other PATHs>`
- (4) `x86_64-unknown-linux-gnu` host toolchain を入れる (通常の Rust install 手順)  
    ```console
    $ curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
    ``` 
- (5) rustup で `x86_64-pc-windows-gnullvm` を追加  
    ```console
    $ rustup target add x86_64-pc-windows-gnullvm
    ```

Rust の cross compile は以下のようにする。

```console
$ cargo build --target x86_64-pc-windows-gnullvm
```

これで Windows 版の .exe が生成される。
