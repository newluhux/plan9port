#include <u.h>
#include <libc.h>
#include <bio.h>

/*
用于表示一个Stardict字典的结构
*/

struct Stardict_dict {
	Biobuf *info;
	Biobuf *index;
	Biobuf *data;
};
typedef struct Stardict_dict Stardict_dict;

/*
使用参数中的文件路径打开一个字典
如果成功则返回一个 Stardict_dict *
如果失败则返回nil
*/

Stardict_dict *stardict_open(char *index,char *data,char *info);

/*
关闭一个字典
如果成功则返回 1
如果失败则返回 负数
*/

int stardict_close(Stardict_dict *stardict);

/*
用于表示Stardict目录文件中的记录
*/

struct Stardict_index {
	char *word;
	u32int offset;
	u32int size;
};
typedef struct Stardict_index Stardict_index;

/*
顺序读出目录文件中的记录
如果成功则返回一个 Stardict_index *
如果已到目录文件末尾或发生错误则返回 nil
*/

Stardict_index *stardict_readindex(Stardict_dict *dict);

/*
释放内存中的 Stardict_index *
如果成功则返回 1
如果失败则返回 负值
*/

int stardict_freeindex(Stardict_index *index);

/*
从字典数据文件中读入数据，依据是传入的Stardict_index记录
如果成功则返回 void * 指向读入内存的数据
如果失败则返回 nil
*/

void *stardict_loaddata(Stardict_dict *dict,Stardict_index *index);

