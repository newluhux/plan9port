#include <u.h>
#include <libc.h>
#include <bio.h>
#include <sdm.h>

#define DEFAULT_DATA "/usr/local/plan9/stardict/default.dict"
#define DEFAULT_INFO "/usr/local/plan9/stardict/default.ifo"
#define DEFAULT_INDEX "/usr/local/plan9/stardict/default.idx"

void main(int argc, char *argv[]) {
	Stardict_dict *dict;
	Stardict_index *index;
	void *data;

	char *pattern;

	if (argc < 2) {
		fprint(2,"Usage: %s word ...\n",argv[0]);
		exits("usage");
	}

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

	int i;

	for (i=1;i<argc;i++) {
		pattern = argv[i];
		Bseek(dict->index,0,0);
		while ((index = stardict_readindex(dict)) != nil) {
			if (strlen(index->word) != strlen(pattern)) {
				goto skip;
			}
			if (strcmp(index->word,pattern) == 0) {
				data = stardict_loaddata(dict,index);
				if (data != nil) {
					write(1,data,index->size);
					free(data);
				}
			}
skip:
			stardict_freeindex(index);
		}
	}
	stardict_close(dict);
	return;
}
