## How to make a tiny-render?
本来想寒假学习一下OpenGL的，发现了这个非常棒的[repository](https://github.com/ssloy/tinyrenderer/wiki)，决定学习这个repository，从而自己也实现一个迷你的渲染器，里面的.obj文件都是官方的，主要学习的是渲染器实现的原理，并且在实现过程中碰到了很多c++的特性，顺带着把c++也巩固了一番。

## 《原理篇》和《语法篇》
我会将这个过程中学习到的新东西，分享给大家。
学习的知识分为2种，一个是渲染器的原理，我将它归类为《原理篇》；
在这个实现的过程中，会将其中的c++的知识总结归纳出来，我将其归类为《语法篇》。

### 原理篇
原理篇的博客文章在[这里](https://www.jianshu.com/nb/42883120)
我首先对每篇教程进行了全文翻译，唯二的区别是博客里的图片都是我自己实现后跑出来的；为了大家可以全局地理解，我会在每篇博客的开头，讲述这篇的原理主要实现了什么。

学习原理的过程中，很多知识点也搞不清楚，很多的解惑都来自于这个[网站](https://learnopengl-cn.github.io/intro/)。

原理篇的博客主要分为——
- [Bresenham直线生成算法和线框渲染](https://www.jianshu.com/writer#/notebooks/42883120/notes/58635995/preview)
- [三角形栅格化和后向面剔除](https://www.jianshu.com/writer#/notebooks/42883120/notes/58891670/preview)
- [隐藏面移除(z缓冲区)](https://www.jianshu.com/writer#/notebooks/42883120/notes/58912800/preview)
- [透视投影](https://www.jianshu.com/writer#/notebooks/42883120/notes/58955265/preview)和[小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/63616949/preview)
- [移动的相机](https://www.jianshu.com/writer#/notebooks/42883120/notes/60595619/preview)
- [软件渲染器的着色器](https://www.jianshu.com/writer#/notebooks/42883120/notes/60598409)
- [TGA文件的读取及编码](https://www.jianshu.com/writer#/notebooks/42883120/notes/60977192)
- [.OBJ文件的理解](https://www.jianshu.com/writer#/notebooks/42883120/notes/62246297/preview)
- [双切线空间法线贴图](https://www.jianshu.com/writer#/notebooks/42883120/notes/64674894/preview)
- [阴影贴图](https://www.jianshu.com/writer#/notebooks/42883120/notes/64708089/preview)
- [环境光遮蔽](https://www.jianshu.com/writer#/notebooks/42883120/notes/64871039/preview)
- [具有透视变形的线性插值技术难点](https://www.jianshu.com/writer#/notebooks/42883120/notes/64877925/preview)
- [实现渲染器的易错点](https://www.jianshu.com/writer#/notebooks/42883120/notes/64878719/preview)

### 语法篇
语法篇的博客文章在[这里](https://www.jianshu.com/nb/41885121)
- [size_t，this指针，初始化列表以及操作符重载](https://www.jianshu.com/writer#/notebooks/41885121/notes/59778915/preview)
- [文件处理及TGA文件的使用](https://www.jianshu.com/writer#/notebooks/41885121/notes/59779157/preview)
- [vector的使用](https://www.jianshu.com/writer#/notebooks/41885121/notes/59779228/preview)
- [template模板的使用](https://www.jianshu.com/writer#/notebooks/41885121/notes/60604475/preview)
- [new，delete的使用](https://www.jianshu.com/writer#/notebooks/41885121/notes/60962619)
- [int argc, char *argv[]的意义](https://www.jianshu.com/writer#/notebooks/41885121/notes/62262746/preview)
