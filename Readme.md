# What's this?

This tool is a command line shell that enables you to edit `.fnt` file directly.

You can use it to dump, edit, merge fnt files, or rename `.png` files it referenced.


[Release download](https://github.com/frto027/FntEditTool/releases)

# 如何构建

~~程序目前只有专门的用途，并没有编写特别的接口，但可以修改main函数实现其它功能。~~

注意，此程序对编译器的字节对齐行为是敏感的。

需要使用VisualStudio2019及MSVC来进行构建。需要确保编译器支持`#pragma pack(push,1)`这个特性，这样才能通过main函数开头的检查。

只要能够确保字节对齐是正确的，也可以尝试使用linux等环境进行构建，`gcc`、`mingw-gcc`亦可。
