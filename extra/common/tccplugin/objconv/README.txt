objconv可将tcc产生的elf格式的object转为coff格式，以供gcc和msvc链接使用
（具体使用方法见notes20250628.txt）



软件主页：
https://agner.org/optimize/#objconv

下载地址（含源代码）：
https://agner.org/optimize/objconv.zip

SHA1校验和：
0d9cd00fcd467404dfe5a3cb0f265d9612fb6536  objconv.zip

SHA256校验和：
0c5ca8fbc7ef1c4afe3cc7cc9018711a5885afacbe8bcebc48028d1ec90ccc7c  objconv.zip



stdcall.patch：
使objconv能正确处理tcc产生的stdcall调用约定的函数的符号名的前置下划线，从而能让objconv与tcc配合使用

vc70.patch：
使objconv能在vc++7.0下通过编译（其它编译器不需要此patch）

vc71.patch：
使objconv能在vc++7.1下通过编译（其它编译器不需要此patch）
