MilkCat
=======

MilkCat是一个简单、高效的中文自然语言处理的工具包，包含分词、词性标注、命名实体识别、新词发现等常用功能。采用C++编写，并提供Python, Go等各种语言的接口。

现在处于*dogfood*阶段 - 欢迎尝试

安装
----

```sh
wget http://milkcat.qiniudn.com/MilkCat-master.tar.gz
tar xzvf MilkCat-master.tar.gz && cd milkcat-0.1
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

Python语言例子
--------------

pymilkcat安装包以及安装方法参见 [milkcat-python](https://github.com/milkcat/milkcat-python)

```python
>>> import pymilkcat
>>> pymilkcat.seg('这个是一个简单的例子')
['这个', '是', '一个', '简单', '的', '例子']
```

C语言例子
---------

```c
// example.c

#include <stdio.h>
#include <milkcat.h>

int main() {
    milkcat_model_t *model = milkcat_model_new(NULL);
    milkcat_t *analyzer = milkcat_new(model, DEFAULT_ANALYZER);
    milkcat_cursor_t *cursor = milkcat_cursor_new(); 

    const char *text = "接着门铃响了，我睁开双眼，转了转门把手。";

    milkcat_item_t item;
    milkcat_analyze(analyzer, cursor, text);
    while (milkcat_cursor_get_next(cursor, &item)) {
        printf("%s/%s  ", item.word, item.part_of_speech_tag);
    }
    printf("\n");

    milkcat_cursor_destroy(cursor);
    milkcat_destroy(analyzer);
    milkcat_model_destroy(model);

    return 0;
}

```

使用gcc编译运行即可

```sh
$ gcc -pthread -o milkcat_demo example.c -lmilkcat
$ ./milkcat_demo
接着/AD  门铃/NN  响/VV  了/AS  ，/PU  我/PN  睁/VV  开/VV  双眼/CD  ，/PU  转/VV  了/AS  转/VV  门把手/NN  。/PU
```


    
