MilkCat
=======

MilkCat是一个简单、高效的中文自然语言处理的工具包，包含分词、词性标注、命名实体识别、新词发现等常用功能。采用C++编写，支持多线程，并提供Python, Go等各种语言的接口。

现在处于*dogfood*阶段 - 欢迎尝试

安装
----

```sh
wget http://milkcat.qiniudn.com/MilkCat-master.tar.gz
tar xzvf MilkCat-master.tar.gz && cd milkcat-0.2
./configure
make && make install
```

运行
----

安装完成之后可以在命令行使用，比如

```sh
$ milkcat corpus.txt
```

对corpus.txt进行分词和词性标注

```sh
$ milkcat -m crf_seg corpus.txt
```

用CRF分词模型对corpus.txt进行分词。

速度
----

* 默认分词(Bigram分词&CRF未登录识)+词性标注(HMM&CRF) 2MB/s
* 默认分词 2.5MB/s
* CRF分词 1.1MB/s
* Bigram分词 3.5MB/s
* 关键词提取 1.5MB/s ~ 2.5MB/s [1]
* 新词发现 2.6MB/s [2]

CPU: AMD Athlon(tm) II X2 260 Processor, 以上数据均为单线程运行速度。

[1] 速度由文本大小情况决定  
[2] 此项测试在2012款Macbook Air(Core i5)中完成，4线程


Python语言例子
--------------

pymilkcat安装包以及安装方法参见 [milkcat-python](https://github.com/milkcat/milkcat-python)

```python
>>> import pymilkcat
>>> pymilkcat.seg('这个是一个简单的例子')
['这个', '是', '一个', '简单', '的', '例子']
```

C++语言例子
---------

从版本0.3开始起程序API从C转变至C++，C API将与Python类似作为语言支持实现。

```c
// example.cc
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser *parser = Parser::New();
  Parser::Iterator *it = parser->Parse("这个是MilkCat的简单测试。");

  while (it->HasNext()) {
    printf("%s/%s  ", it->word(), it->part_of_speech_tag());
    it->Next();
  }

  parser->Release(it);
  delete parser;
  return 0;
}

```

使用g++编译运行即可

```sh
$ g++ -o milkcat_demo example.cc -lmilkcat
$ ./milkcat_demo
这个/PN  是/VC  MilkCat/NN  的/DEG  简单/JJ  测试/NN  。/PU
```

