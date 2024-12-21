<!--
SPDX-FileCopyrightText: 2024 Hanabusa Masahiro
SPDX-License-Identifier: CC-BY-4.0
-->
# Windows でも UTF-8 で code を書きたい

Linux も Mac も UTF-8 なので、Windows target も UTF-8 で C/C++ code を書きたい。
以下の3点を実施すれば、UTF-8 で動作するようになる。

- C/C++ source code の encoding を UTF-8 にする
- Win32 API を UTF-8 encoding で呼び出せるように、manifest を追加
- `printf()` などが化けないように、console code page を指定

最小限の sample が [windows_build_tips/hello_utf8](https://github.com/hanabu/windows_build_tips/tree/main/hello_utf8) に入っている。

## C/C++ source code を UTF-8 で書く

LLVM MinGW の Clang 利用の場合、そもそも Clang が UTF-8 しか受け付けないので、何も考えずに UTF-8 encoding で source code を書けばよい。

MSVC の場合、default では OS の code page を仮定する。例えば日本語環境で MSVC を走らせると ShiftJIS encoding として扱われる。
Source code を UTF-8 で書くためには以下のどちらかを実施。

- BOM (byte order mark) をつけた UTF-8-BOM 形式で source code を保存。Visual Studio 2013 あたりから対応されている。
- Compiler option に '/utf-8' をつける。Visual Studio 2015 から対応されている。

## Win32 API を UTF-8 encoding で呼び出す

Windows 10 以降であれば、multi byte 版の Win32 API に UTF-8 文字列を渡すことができるようになっている。
ただし、default では 日本語環境なら ShiftJIS という具合に、過去との互換を優先した encoding で動作するため、assembly manifest を書いて UTF-8 で動作することを明示的に宣言する必要がある。

まず manifest の XML を書き、`utf8_manifest.xml` のような名前で保存する。BOM つき UTF-8 での保存が必要。

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly manifestVersion="1.0" xmlns="urn:schemas-microsoft-com:asm.v1">
<application>
<windowsSettings>
<activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings">UTF-8</activeCodePage>
</windowsSettings>
</application>
</assembly>
```

続いて、resource file (`*.rc`) に追記する。`24` が manifest 用の resource type 番号。

```rc
// Application manifest for UTF-8 code page
1 24 "utf8_manifest.xml"
```

詳しくは [Use UTF-8 code pages in Windows apps](https://learn.microsoft.com/windows/apps/design/globalizing/use-utf8-code-page) などを参照。

## Console code page を指定

`printf()` などで console 表示する場合、console の code page も UTF-8 用の code page 65001 に切り替える必要がある。
最小限は `SetConsoleOutputCP()` を 1度呼ぶだけだが、行儀よく行うなら終了時に元の code page に戻しておくとよい。

```.c
// 最小限
SetConsoleOutputCP(CP_UTF8);
```

行儀よく行う例:

```.c
// Set console code-page as UTF-8
const UINT original_cp = GetConsoleOutputCP();
SetConsoleOutputCP(CP_UTF8);

// Application logic
printf("hello\n");

// Restore console code page on exit
SetConsoleOutputCP(original_cp);
return 0;
```

