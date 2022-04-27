### 初始化开发环境

对于 node-c++ 插件程序，本代码仓未采用官方的 node-gyp 编译构建方式，而是通过逆向分析其构建输出内容，找到核心依赖项和相关编译选项，直接用 clang / gcc 命令编译构建动态库（.node 插件就是系统动态库），这样既便于初学者快速上手、做学习实践和技术验证，也便于和常用的 c++ 构建工具融合（如 cmake 与 gn 等），以便在现有的中大型 c++ 项目上快速为 node 提供 js 接口。

因此，我把相关环境和依赖项整合到一起（整合方法附在文后，可参考配置不同版本），构成了本仓的预置开发环境，为便于提交和下载，归档在 node.zip 这个压缩包中，使用前请解压到名为 node 的子目录中。除此之外，还要求系统预装的环境是：

- clang 12 及以上版本

- windows 系统请安装 MSVC 构建工具（为更好支持 C++ 新特性，建议安装 2017 及以上版本）

### 运行 node-c++ 插件程序案例

在 windows 系统上，用 powershell 执行每个案例目录下的`compile.ps1`脚本即可。