#include <u.h>
#include <libc.h>
#include <bio.h>
#include <sdm.h>

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

	i = 0;
	word[i] ='\0';

loopgetc:
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
		}
		word[i++] = c;
	}
	if (word[i] != '\0' && i < 256) {
		word[i+1] = '\0';
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
