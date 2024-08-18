CPK 解包工具

先将 uncpk.c 编译为 32 位的 uncpk.exe （例如：CL /O2 uncpk.c）
然后将 uncpk.exe 和 uncpk_PAL3.bat / uncpk_PAL3A.bat 放置于游戏目录下
运行 uncpk_PAL3.bat / uncpk_PAL3A.bat 即可将所有游戏 CPK 解包到正确的目录下

如果游戏文件有只读属性，请提前去除，否则解包工具会因为无法覆盖只读文件而报错
为了避免文件名乱码，简体版游戏请在简体系统下解包，繁体版游戏请在繁体系统下解包
建议解包时关闭 Microsoft Defender 实时保护，以提高解包速度
