// SPDX-License-Identifier: CC-BY-4.0
// SPDX-FileCopyrightText: 2024 Hanabusa Masahiro
// 
// UTF-8 print sample
//
#include <stdio.h>
#include <windows.h>

int main(int argc, char* argv[])
{
	// Set console code-page as UTF-8
	const UINT original_cp = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);

    printf("你好 / こんにちは / 안녕하세요\n");
    MessageBox(NULL, "你好 / こんにちは / 안녕하세요", "Hello", MB_OK);

	// Restore console code page
	SetConsoleOutputCP(original_cp);

	return 0;
}

