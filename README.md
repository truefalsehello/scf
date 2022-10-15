# scf

#### 介绍
一个完全自己编写的编译器框架

#### 软件架构
软件架构说明


#### 安装教程

1.  用git下载到本机，
2.  在scf/parse目录下直接执行make，获得编译器的可执行文件scf，

3.  写一段示例代码，例如：
int printf(const char* fmt);
int main()
{
    printf("hello world\n");
    return 0;
}
保存为文件hello.c，

4，然后用scf编译它，命令为：scf hello.c
即可获得编译后的可执行文件，默认文件名是1.out

5，给它加上可执行权限：chmod +x 1.out

6，执行它：./1.out
即可看到打印的"hello world"

7，源代码的扩展名建议用.c，虽然编译器只会把它当成文本文件，但.c在大多数"编辑器"里都有"语法提示"。

8，scf对源文件扩展名的检测在main.c里，你可以把第66行的.c改成任何你想要的扩展名:( 但不能是.a,.so,.o.

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
