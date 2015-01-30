MilkCat
=======

MilkCat是由C++实现的开源依存句法分析库，自带中文分词以及词性标注，当然也可以作为分词和词性标注库来使用。MilkCat除了提供C++ API本身外，还支持C、python、Go等多种语言的接口。

现在处于beta阶段

模型以及性能
----------

### 分词

BIGRAM: 3.5MB/s, F=94.6% (bakeoff2005 MSRA)  
CRF: 1.5MB/s, F=96.6%   
BIGRAM+CRF未登录词识别: 2.0MB/s

### 词性标注

CRF: 61.5k word/s, TA=93.9% (CTB8)

### 依存句法分析

Yamada: 45.5k word/s, UAS=82.9 (CTB5)  
Beam Yamada: 8.4k word/s, UAS=84.9 (CTB5)

在线演示
-------

[演示地址](http://milk.cat)

下载
----

```sh
wget http://milk.cat/milkcat-0.4.tar.gz
```

或者

```sh
wget http://milkcat.qiniudn.com/milkcat-0.4.tar.gz
```

安装
----

```sh
tar xzvf milkcat-0.4.tar.gz && cd milkcat-0.4
./configure
make && make install
```

运行
----

安装完成之后可以在命令行使用

```sh
$ milkcat corpus.txt
```

对corpus.txt进行分词和词性标注

```sh
$ milkcat -m crf_seg corpus.txt
```

用CRF分词模型对corpus.txt进行分词。

Python例子
--------------

pymilkcat安装包以及安装方法参见 [pymilkcat](https://github.com/milkcat/pymilkcat)

```python
>>> import pymilkcat
>>> parser = pymilkcat.Parser()
>>> parser.pos_tag('这个是MilkCat的简单测试。')
```

[('这个', 'PN'), ('是', 'VC'), ('MilkCat', 'NN'), ('的', 'DEG'), ('简单', 'JJ'), ('测试', 'NN'), ('。', 'PU')]


C++例子
---------

从版本0.3开始起程序API从C转变至C++，C API将与Python类似作为语言支持实现。

```c
// example.cc
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser *parser = Parser::New();
  Parser::Iterator *it = new Parser::Iterator();
  parser->Predict(it, "我的猫喜欢喝牛奶。");

  while (!it->End()) {
    printf("%s/%s  ", it->word(), it->part_of_speech_tag());
    it->Next();
  }
  puts("");

  delete it;
  delete parser;
  return 0;
}

```

使用g++编译运行即可

```sh
$ g++ -o milkcat_demo example.cc -lmilkcat
$ ./milkcat_demo
我/PN  的/DEG  猫/NN  喜欢/VV  喝/VV  牛奶/NN  。/PU
```

