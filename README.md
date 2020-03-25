# How to make a tiny-render?
这次通过学习这个[repository](https://github.com/ssloy/tinyrenderer/wiki)的内容，自己也实现了一个迷你渲染器。

实现的结果如下：

![图1](https://upload-images.jianshu.io/upload_images/20254147-e5c91011a640bbd4.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
![图2](https://upload-images.jianshu.io/upload_images/20254147-81cce39e95ac9b30.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
![图3](https://upload-images.jianshu.io/upload_images/20254147-5d863ad51f3a2351.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
![图4](https://upload-images.jianshu.io/upload_images/20254147-62d864ae2226eadc.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
![图5](https://upload-images.jianshu.io/upload_images/20254147-a6394363d26443ad.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

本次复现主要的收获是**渲染器实现**的工作原理，并且在实现过程中将**Cpp**也巩固了一番。

---

## 文件介绍

1. 《tiny render》文件夹中《Lesson 1~Lesson 8》是在学习博客中写的。
2. 《tiny render》文件夹中《Final render》是最后自己实现的。

---

## 《原理篇》和《语法篇》
我决定将这个过程中学习到的新东西，分享给大家。

学习的知识分为2种，一个是渲染器的原理，我将它归类为**原理篇**；

在这个实现的过程中，会将其中的c++的知识总结归纳出来，我将其归类为**语法篇**。

### 原理篇
我首先对每篇教程进行了**全文翻译**，唯二的区别是博客里的图片都是我自己实现后跑出来的；为了大家可以全局地理解，我会加上每一章对应的**小总结**。

学习原理的过程中，很多知识点也搞不清楚，自己也google了很多的外部资料，其中很多的**解惑**都来自于这个[网站](https://learnopengl-cn.github.io/intro/)。

原理篇的博客主要分为——
- [Bresenham直线生成算法和线框渲染](https://www.jianshu.com/writer#/notebooks/42883120/notes/58635995/preview)和[Lesson 1 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65560072/preview)
- [三角形栅格化和后向面剔除](https://www.jianshu.com/writer#/notebooks/42883120/notes/58891670/preview)和[Lesson 2 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65559564/preview)
- [隐藏面移除(z缓冲区)](https://www.jianshu.com/writer#/notebooks/42883120/notes/58912800/preview)和[Lesson 3 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65557384/preview)
- [透视投影](https://www.jianshu.com/writer#/notebooks/42883120/notes/58955265/preview)和[Lesson 4 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/63616949/preview)
- [移动的相机](https://www.jianshu.com/writer#/notebooks/42883120/notes/60595619/preview)和[Lesson 5 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65534255/preview)
- [软件渲染器的着色器](https://www.jianshu.com/writer#/notebooks/42883120/notes/60598409)和[Lesson 6 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65485485/preview)
- [TGA文件的读取及编码](https://www.jianshu.com/writer#/notebooks/42883120/notes/60977192)
- [.OBJ文件的理解](https://www.jianshu.com/writer#/notebooks/42883120/notes/62246297/preview)
- [双切线空间法线贴图](https://www.jianshu.com/writer#/notebooks/42883120/notes/64674894/preview)和[Lesson 6 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65402522/preview)
- [阴影贴图](https://www.jianshu.com/writer#/notebooks/42883120/notes/64708089/preview)和[Lesson 7 小总结](https://www.jianshu.com/writer#/notebooks/42883120/notes/65374809/preview)
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

---

### 联系我

当然，大家有任何问题都可以与我联系。

我的**邮箱**：

**Outlook**: hlhdsteven825@outlook.com

**Gmail**: stevenhd19970825@gmail.com
