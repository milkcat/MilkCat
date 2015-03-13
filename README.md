# MilkCat

MilkCat是由C++实现的开源依存句法分析库，自带中文分词以及词性标注，当然也可以作为分词和词性标注库来使用。MilkCat除了提供C++ API本身外，还支持C、python、Go等多种语言的接口。

现在处于beta阶段

# 模型以及性能

### 分词

BIGRAM: 3.5MB/s, F=94.6% (bakeoff2005 MSRA)  
CRF: 1.5MB/s, F=96.6%   
BIGRAM+CRF未登录词识别: 2.0MB/s

### 词性标注

CRF: 61.5k word/s, TA=93.9% (CTB8)

### 依存句法分析

Yamada: 45.5k word/s, UAS=82.9 (CTB5)  
Beam Yamada: 8.4k word/s, UAS=84.9 (CTB5)

# 在线演示

[演示地址](http://milk.cat)

# MilkCat/C++使用简介

1. [下载和安装](#download-and-install)
3. [Hello, World](#hello-world)
4. [配置Parser](#options)
    1. [配置依存分析](#options-parsing)
    2. [配置模型路径](#options-model-path)
    3. [配置用户词典](#options-userdict)
    4. [Parser::Options的成员函数](#options-member-functions)
5. [问题反馈](#bugs)
    

# 下载和安装 <a id="download-and-install"></a>

## 下载

### Linux / Mac OS X

对于Linux或者Mac OS X来说，可以通过下载MilkCat的源代码包，然后进行编译安装。

目前可以从三个地方进行下载：

```sh
wget http://milkcat.qiniudn.com/milkcat-0.4.tar.gz
OR
wget http://milk.cat/milkcat-0.4.tar.gz
OR
wget https://github.com/milkcat/MilkCat/releases/download/v0.4-beta.1/milkcat-0.4.tar.gz
```

### 历史版本

需要下载历史版本时可前往[MilkCat的Release页面](https://github.com/milkcat/MilkCat/releases)

## 安装

### Linux / Mac OS X

MilkCat除了依赖于标准C++库外并未以来其他额外的库，因此只需确保计算机中安装有C++的编译环境即可编译安装

```sh
tar xzvf milkcat-0.4.tar.gz && cd milkcat-0.4
./configure
make && sudo make install
```

安装后运行

```sh
$ echo '我的猫喜欢喝牛奶' | milkcat -i
```

进行测试，如果正确出现

```sh
我/PN  的/DEG  猫/NN  喜欢/VV  喝/VV  牛奶/NN
```

即代表安装成功。

# Hello, World! <a id="hello-world"></a>

MilkCat的Hello World!程序如下

```c++
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser parser;
  Parser::Iterator it;

  if (parser.ok()) {
    parser.Predict(&it, "我的猫喜欢喝牛奶。");
    while (it.Next()) {
      printf("%s/%s  ", it.word(), it.part_of_speech_tag());
    }
    putchar('\n');
  } else {
    printf("error: %s\n", milkcat::LastError());
  }
  return 0;
}
```

将以上文件保存为example01.cc，使用g++编译运行

```sh
$ g++ -o example01 example01.cc -lmilkcat
$ ./example01
```

运行结果

```sh
我/PN  的/DEG  猫/NN  喜欢/VV  喝/VV  牛奶/NN  。/PU
```

## 解释

在使用MilkCat之前，首先需要创建milkcat::Parser类。

```c++
Parser parser;
```

因为MilkCat不使用C++的异常处理机制，需要使用`parser.ok() == true`判断创建是否成功。如果创建失败，可以使用`LastError()`获得出错的信息。

```c++
Parser::Iterator it;
```

`Parser::Iterator`是保存预测结果的迭代器，`it`用来迭代得到所有的分析结果。

```c++
parser.Predict(&it, "我的猫喜欢喝牛奶。");
```

在创建完Parser和Parser::Iterator后，就可以对句子进行分析了，Predict方法根据句子"我的猫喜欢喝牛奶。"，预测出它最有可能的分析结果并且将此结果存入迭代器it内。

```c++
while (it.Next()) {
  printf("%s/%s  ", it.word(), it.part_of_speech_tag());
}
```

最终通过while循环迭代输出每一个词语即可。Iterator::Next()是切换到下一个词语并且返回true，当迭代结束时返回false。此外，word()表示的是当前的词语，而part_of_speech_tag()表示的是当前的词性。

Iterator的成员函数包括:

| 函数名 |描述| 
|:----------|:--|
| bool Next() | Iterator::Next()是切换到下一个词语，存在下一个词语时返回true，不存在返回false |
| const char *word() const | 得到当前词语 |
| const char *part_of_speech_tag() const | 得到当前词语的词性 |
| int head() const | 得到当前词语在依存树中的依存节点（父节点），节点序号从`1`开始，`0`是根节点 |
| const char *dependency_label() const | 得到当前词语在依存树中与依存节点的依存关系 |
| int type() const | 得到当前词语的类型，类型定义于`enum Parser::WordType;`中 |
| bool is_begin_of_sentence() const | 当前词语是否为句子起始词语（第一个词语），是则返回true，不是返回false，可用于句子切分 |

## 需要注意的地方

1. it.word(), it.part_of_speech_tag()等函数所返回的指针由Parser内部维护，不需要用户自己删除。但是仅仅保证在下一个Iterator::Next()调用之前指针的内容是有效的，因此需要在Next()调用之前将指针的内容复制出来（strcpy或者直接使用std::string）。

2. 需要避免重复创建/删除Parser::Iterator，因为Parser::Iterator创建时开销比较大。当要分析多个句子时，Parser::Iterator可重复使用，如：

```c++
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;
const char text[][128] = {
  "我的猫喜欢喝牛奶。",
  "明天也是晴天。"
};

int main() {
  Parser parser;
  Parser::Iterator it;

  for (int i = 0; i < 2 && parser.ok(); ++i) {
    parser.Predict(&it, text[i]);
    while (it.Next()) {
      printf("%s/%s  ", it.word(), it.part_of_speech_tag());
    }
    putchar('\n');
  }

  return 0;
}
```

# 配置Parser <a id="options"></a>

在MilkCat中Parser所使用的模型、编码、数据文件等配置非常灵活，可以由用户自行指定，其中默认的配置为BIGRAM+CRF混合模型分词，HMM+CRF混合模型词性标注，不进行依存分析，使用UTF-8编码。

所有的配置均可在`Parser::Options`类中找到，然后将其作为参数传入`Parser`的构造函数中即可，如

```C++
Parser::Options options;
options.UseCRFSegmenter();
options.UseCRFPOSTagger();
options.UseBeamYamadaParser();

Parser parser(options);
```

上述代码即配置了CRF分词、CRF词性标注以及Beam Yamada依存分析的Parser。

## 配置依存分析 <a id="option-parsing"></a>

上述代码中简单地介绍了配置依存分析的例子，以下是完整的使用MilkCat进行依存分析的代码

```C++
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser::Options options;
  options.UseCRFSegmenter();
  options.UseCRFPOSTagger();
  options.UseBeamYamadaParser();
  
  Parser parser(options);
  Parser::Iterator it;
  if (parser.ok()) {
    parser.Predict(&it, "我的猫喜欢喝牛奶。");

    while (it.Next()) {
      printf("%s\t%s\t%d\t%s\n",
             it.word(),
             it.part_of_speech_tag(),
             it.head(),
             it.dependency_label());
    }
  } else {
    printf("error: %s\n", milkcat::LastError());
  }

  return 0;
}
```

## 配置模型路径 <a id="options-model-path"></a>

MilkCat分析预测过程中需要使用到一些训练好的模型数据，比如用于分词的CRF模型数据或者用于依存分析的感知器模型数据等。默认的模型路径即MilkCat在编译安装时所有模型所保存的路径。对于Linux/Mac OS X来说模型路径在大多数情况下是/usr/local/share/milkcat。

同样也可以使用用户自定义的模型路径，使用

```c++
void Options::SetModelPath(const char *model_path);
```

函数进行配置。

完整的代码如下，使用自定义的模型路径`./milkcat-data/`

```c++
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser::Options options;
  options.SetModelPath("./milkcat-data");

  Parser parser(options);
  Parser::Iterator it;
  if (parser.ok()) {
    parser.Predict(&it, "我的猫喜欢喝牛奶。");
    while (it.Next()) {
      printf("%s/%s  ", it.word(), it.part_of_speech_tag());
    }
    putchar('\n');
  } else {
    printf("error: %s\n", milkcat::LastError());
  }
  return 0;
}
```

在运行前需要先将模型文件夹复制到当前路径下的milkcat-data下

```sh
$ cp -r /usr/local/share/milkcat/ ./milkcat-data
$ g++ -o example3_demo example3_demo.cc -lmilkcat
$ ./example3_demo
```

## 配置用户词典 <a id="options-userdict"></a>

用户词典保存用户自定义的一些词语。在分词之前，将这些词语加入分词词库中，MilkCat分词模块就可以使用到。

用户词典是一个文本文件，比如

    博丽灵梦
    雾雨魔理沙
    十六夜咲夜 2.0

上面的用户词典例子中，每一行代表一个词语，一共有两种形式：第一种就是单独一个词语，如博丽灵梦和雾雨魔理沙。第二种就是"词语<空格>权值"的形式，如第三行的"十六夜咲夜 2.0"。

在此，权值越小代表越有可能是词语，权值越大代表这个越不可能是词语，当权值大于一定的上限时，这个词语就无法被切分出来。

可以使用

```c++
void Options::SetUserDictionary(const char *model_path);
```

配置使用用户自定义词典。

以下例子首先使用`Options::SetUserDictionary()`函数配置用户词典userdict.txt，然后加载使用。

```c++
#include <milkcat.h>
#include <stdio.h>

using milkcat::Parser;

int main() {
  Parser::Options options;
  options.SetUserDictionary("userdict.txt");

  Parser parser(options);
  Parser::Iterator it;

  if (parser.ok()) {
    parser.Predict(&it, "博丽灵梦、雾雨魔理沙、十六夜咲夜。");
    while (it.Next()) {
      printf("%s  ", it.word());
    }
    putchar('\n');
  } else {
    printf("error: %s\n", milkcat::LastError());
  }
  return 0;
}
```

在运行前先将

    博丽灵梦
    雾雨魔理沙
    十六夜咲夜 2.0

上述内容保存成userdict.txt (注意使用UTF-8编码、Unix换行符形式)，然后编译运行即可。

```sh
$ g++ -o example05_demo example05.cc -lmilkcat
$ ./example05_demo
博丽灵梦  、  雾雨魔理沙  、  十六夜咲夜  。
```

此时如果将userdict.txt中的

    十六夜咲夜 2.0

换成

    十六夜咲夜 200.0

因为权值太大(200.0)，"十六夜咲夜"就不会被切分出来

```sh
$ ./example05_demo
博丽灵梦  、  雾雨魔理沙  、  十六  夜  咲夜  。
```

### 需要注意的地方

1. CRF分词（UseCrfSegmenter），用户词典对其无效。需要使用用户词典的时候可以选择BIGRAM+CRF模型（UseMixedSegmenter）或者BIGRAM分词（UseBigramSegmenter）。

2. 用户词典必须使用UTF-8编码。

## Parser::Options的成员函数 <a id="options-member-functions"></a>

Parser::Options的成员函数如下，可以通过这些函数配置Parser：

| 函数 | 描述 |
|:--|:--|
| void UseGBK() | 分析GBK编码的句子 |
| void UseUTF8() | 分析UTF-8编码的句子 |
| void UseMixedSegmenter() | 使用BIGRAM和CRF混合的分词模型，此模型首先使用BIGRAM模型进行分词，其后使用CRF模型做未登录词识别。保证速度的前提下，效果较好，推荐使用。 |
| void UseCRFSegmenter() | 使用CRF（条件随机场）分词模型，未登录词召回好，但是速度略慢 |
| void UseUnigramSegmenter() | 使用UNIGRAM（一元语法）分词模型，速度最快，但是歧义处理能力不足 |
| void UseBigramSegmenter() | 使用BIGRAM（二元语法）分词模型，速度比CRF以及MIXED模型都快，但是不具有未登录词召回的功能。 |
| void UseMixedPOSTagger() | 使用HMM（隐马尔可夫）以及CRF（条件虽机场）模型进行词性标注，精度略微低于CRF模型，速度介于CRF与HMM之间，推荐使用。 |
| void UseHMMPOSTagger() | 使用HMM（隐马尔可夫）模型进行词性标注，速度最快，但是精度不高 |
| void UseCRFPOSTagger() | 使用CRF（条件虽机场）模型进行词性标注，精度最高，但是速度慢于MIXED模型。 |
| void NoPOSTagger() | 不进行词性标注 |
| void UseYamadaParser() | 使用Yamada（也称作arc-standard）算法进行依存句法分析，特点是O(n)的时间复杂度。 |
| void UseBeamYamadaParser() | 使用Yamada算法+Beam search进行依存句法分析，参考[(Zhang et al, 11)](http://dl.acm.org/citation.cfm?id=2002777)论文中的模型。O(B*n)时间复杂度，其中B是beam的大小，在MilkCat中为8。相比Yamada算法，精度提升2%左右，但是分析所花时间也是Yamada算法的8倍。 |
| void NoDependencyParser() | 不进行依存分析。 |

# 问题反馈 <a id="bugs"></a>

非常欢迎将任何关于MilkCat问题反馈给我，包括编译问题，运行期间的问题，也可以是文档的拼写错误等，我会及时解决。

e-mail: ling032x#gmail.com  
Twitter: @ling0322  
Github: [milkcat/MilkCat](https://github.com/milkcat/MilkCat)

> Written with [StackEdit](https://stackedit.io/).
