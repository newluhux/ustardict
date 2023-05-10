#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#define min(x, y) ((x) < (y) ? (x) : (y))

struct stardict {
	FILE *idx;
	FILE *ifo;
	FILE *dict;
};

#define stardict_offset uint32_t
#define stardict_size uint32_t

#define STARDICT_WORDMAX (255)
#define STARDICT_IDXBUFSIZE (STARDICT_WORDMAX + '\0' + sizeof(stardict_offset)\
				 + sizeof(stardict_size))
#define STARDICT_DICTBUFSIZE BUFSIZ

inline int stardict_strcmp(char *s1, char *s2)
{
	return strcasecmp(s1, s2);
}

int getword(char *buf, int bufsize)
{
	int c;
	int i;
	while ((isspace(c = getchar())) && c != EOF) ;
	if (c == EOF)
		exit(EXIT_SUCCESS);
	ungetc(c, stdin);
	i = 0;
	while ((!isspace(c = getchar())) && (i < (bufsize - 1))) {
		buf[i] = c;
		i++;
	}
	if (c == EOF)
		exit(EXIT_SUCCESS);
	ungetc(c, stdin);
	buf[i] = '\0';
	return i;
}

int getstr(FILE * stream, char *buf, int bufsize)
{
	int i = 0;
	int c;
	while (((c = fgetc(stream)) != EOF) && (c != '\0') &&
	       (i < (bufsize - 1))) {
		buf[i] = c;
		i++;
	}
	ungetc(c, stream);
	buf[i] = '\0';
	return i;
}

FILE *efopen(char *fn, char *mode)
{
	FILE *fp;
	fp = fopen(fn, mode);
	if (fp == NULL) {
		fprintf(stderr, "can't open %s : ", fn);
		perror("");
	}
	return fp;
}

int stardict_open(char *prefixname, struct stardict *s)
{
	char temp[PATH_MAX];
	snprintf(temp, PATH_MAX, "%sidx", prefixname);
	s->idx = efopen(temp, "r");
	snprintf(temp, PATH_MAX, "%sdict", prefixname);
	s->dict = efopen(temp, "r");

	int ret = 0;
	if (s->idx)
		ret++;
	if (s->dict)
		ret++;

	return ret;
}

int main(int argc, char *argv[])
{
	char *prefix;
	if (argc < 2) {
		prefix = "./default";
	} else {
		prefix = argv[1];
	}
	struct stardict dict;
	if (stardict_open(prefix, &dict) != 2) {
		fprintf(stderr, "open dictionary file failed\n");
		exit(EXIT_FAILURE);
	}

	char inputbuf[STARDICT_WORDMAX];
	char idxbuf[STARDICT_IDXBUFSIZE];
	char dictbuf[STARDICT_DICTBUFSIZE];
	stardict_offset offset;
	stardict_size size;
	stardict_size readed;
	stardict_size needr;
	stardict_size nread;
	while (1) {
		while (getword(inputbuf, STARDICT_WORDMAX) <= 0) ;
		rewind(dict.idx);
		while (1) {
			getstr(dict.idx, idxbuf, STARDICT_IDXBUFSIZE);
			offset = 0;
			size = 0;
			fseek(dict.idx, 1, SEEK_CUR);	/* skip '\0' */
			fread(&offset, 1, sizeof(offset), dict.idx);
			fread(&size, 1, sizeof(size), dict.idx);
			offset = htobe32(offset);
			size = htobe32(size);
			if (stardict_strcmp(idxbuf, inputbuf) == 0) {
				printf("%s FOUND\n", inputbuf);
				fseek(dict.dict, offset, SEEK_SET);
				readed = 0;
				while (readed < size) {
					needr = min(size - readed,
						    STARDICT_DICTBUFSIZE);
					nread = fread(dictbuf, 1, needr,
						      dict.dict);
					fwrite(dictbuf, 1, nread, stdout);
					readed += nread;
				}
				putchar('\n');
				break;
			}
			if (feof(dict.idx)) {
				printf("%s NOT FOUND\n", inputbuf);
				break;
			}
		}
	}
}
