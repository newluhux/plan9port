#include <u.h>
#include <libc.h>
#include <bio.h>
#include <sdm.h>

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
