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

static unsigned long
ntohl(int x)
{
	unsigned long n;
	unsigned char *p;

	n = x;
	p = (unsigned char*)&n;
	return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
}

Stardict_dict *stardict_open(char *index, char *data, char *info) {
	Stardict_dict *ret = (Stardict_dict *)malloc(sizeof(Stardict_dict));

	int testfd;

	testfd = open(index,OREAD);
	if (testfd == -1) {
		goto err;
	}
	close(testfd);
	testfd = open(data,OREAD);
	if (testfd == -1) {
		goto err;
	}
	close(testfd);
	testfd = open(info,OREAD);
	if (testfd == -1) {
		goto err;
	}
	close(testfd);


	// 打开文件
	ret->index = Bopen(index,OREAD);
	if (ret->index == nil) {
		goto err;
	}
	ret->data = Bopen(data,OREAD);
	if (ret->data == nil) {
		goto err;
	}
	ret->info = Bopen(info,OREAD);
	if (ret->info == nil) {
		goto err;
	}

	return ret;

err:
	if (ret->index != nil) {
		Bterm(ret->index);
	}
	if (ret->data != nil) {
		Bterm(ret->data);
	}
	if (ret->info != nil) {
		Bterm(ret->info);
	}
	free(ret);
	ret = (Stardict_dict *)nil;
	return ret;
}

int stardict_close(Stardict_dict *stardict) {
	if (stardict->index != nil) {
		Bterm(stardict->index);
	}
	if (stardict->data != nil) {
		Bterm(stardict->data);
	}
	if (stardict->info != nil) {
		Bterm(stardict->info);
	}
	free(stardict);
	
	return 1;
}

Stardict_index *stardict_readindex(Stardict_dict *dict) {
	Stardict_index *ret = (Stardict_index *)malloc(sizeof(Stardict_index));
	ret->word = Brdstr(dict->index,'\0','\0');
	if (ret->word == nil) {
		goto err;
	}

	u32int temp;
	if (Bread(dict->index,&temp,32/8) != 32/8) {
		goto err;
	}
	ret->offset = 0;
	ret->offset += ntohl(temp);
	if (Bread(dict->index,&temp,32/8) != 32/8) {
		goto err;
	}
	ret->size = 0;
	ret->size = ntohl(temp);

	return ret;

err:
	free(ret->word);
	free(ret);
	ret = (Stardict_index *)nil;
	return ret;
}

int stardict_freeindex(Stardict_index *index) {
	free(index->word);
	free(index);

	return 1;
}

void *stardict_loaddata(Stardict_dict *dict,Stardict_index *index) {
	void *ret;
	ret = (void *)malloc(index->size);
	if (Bseek(dict->data,index->offset,0) != index->offset) {
		goto err;
	}

	if (Bread(dict->data,ret,index->size) != index->size) {
		goto err;
	}

	return ret;

err:
	free(ret);
	ret = (void *)nil;
	return ret;
}

#define DEFAULT_DATA "/usr/local/plan9/stardict/default.dict"
#define DEFAULT_INFO "/usr/local/plan9/stardict/default.ifo"
#define DEFAULT_INDEX "/usr/local/plan9/stardict/default.idx"

static char *getword(Biobuf *input) {
	if (input == nil) {
		return nil;
	}
	int c;

	char word[256];
	int i;
loopgetc:

	i = 0;
	word[i] ='\0';


	while (1) {
		c = Bgetc(input);
		if (c == Beof) {
			return nil;
		} else if (c == '\n') {
			break;
		} else if (c == '\t') {
			break;
		} else if (c == ' ') {
			break;
		} else {
			word[i] = c;
			++i;
		}
	}
	if (word[i] != '\0' && i < 256) {
		word[i] = '\0';
	}
	if (strlen(word) < 1) {
		goto loopgetc;
	}
	char *ret;
	ret = strdup(word);
	return ret;
}

void main(int argc, char *argv[]) {
	Stardict_dict *dict;
	Stardict_index *index;
	void *data;
	int from_stdin = 0;
	Biobuf *stdin_bio;

	char *pattern;
	if (argc < 2) {
		from_stdin = 1;
	}

	if (from_stdin == 1) {
		stdin_bio = Bfdopen(0,OREAD);
		if (stdin_bio == nil) {
			fprint(2,"Can't open stdin.\n");
			exits("io error");
		}
	}

	ARGBEGIN {
	case 'h':
		fprint(2,"Usage: sdm word ...\n");
		exits("usage");
	} ARGEND

	char *dictfile = DEFAULT_DATA;
	char *indexfile = DEFAULT_INFO;
	char *infofile = DEFAULT_INDEX;

	char *env;
	env = getenv("stardict_idx");
	if (env != nil) {
		indexfile = env;
	}
	env = getenv("stardict_ifo");
	if (env != nil) {
		infofile = env;
	}
	env = getenv("stardict_dict");
	if (env != nil) {
		dictfile = env;
	}

	dict = stardict_open(indexfile,dictfile,infofile);
	if (dict == nil) {
		fprint(2,"Can't open file.\n");
		exits("io error");
	}

	while (1) {
		if (from_stdin == 1) {
			pattern = getword(stdin_bio);
			if (pattern == nil) {
				break;
			}
		} else if (*argv) {
			pattern = *argv++;
		} else {
			break;
		}

		if (!pattern) {
			break;
		}
		Bseek(dict->index,0,0);
		while ((index = stardict_readindex(dict)) != nil) {
			if (strlen(index->word) != strlen(pattern)) {
				goto skip;
			}
			if (strcmp(index->word,pattern) == 0) {
				data = stardict_loaddata(dict,index);
				if (data != nil) {
					write(1,data,index->size);
					write(1,"\n",1);
					free(data);
				}
			}
skip:
			stardict_freeindex(index);
		}
		if (from_stdin == 1) {
			free(pattern);
		}
	}
	stardict_close(dict);
	return;
}
