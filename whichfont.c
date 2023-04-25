#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include<wchar.h>
#include<locale.h>
#include<unistd.h>
#include<getopt.h>

enum {
		OP_NONE = 0,
		OP_ALL,
		OP_SORT,
		OP_HELP,
		OP_END
};

char** whichfont(long int unicodepoint, char* argv[], int k_optind, int ops){
	FcPattern *pattern;
	FcCharSet *charset;
	FcObjectSet	*os = 0;
	const FcChar8 *format = NULL;
	
	
	pattern = FcPatternCreate();
	if (!pattern)
	{
		printf ("Unable to parse the pattern\n");
		return NULL;
	}
	charset = FcCharSetCreate();
	
	FcCharSetAddChar(charset, (FcChar32) unicodepoint);
	FcPatternAddCharSet(pattern, FC_CHARSET, charset);

	while (argv[++k_optind])
	{
		if (!os)
		{
			os = FcObjectSetCreate ();
		}
		FcObjectSetAdd (os, argv[k_optind]);
	}

	FcConfigSubstitute (0, pattern, FcMatchPattern);
    	FcDefaultSubstitute (pattern);

	FcFontSet *fs;
	fs = FcFontSetCreate ();

	if(ops){
		//with -a or -s
		FcResult font_result; //error handling if any, so we need this font_result
		FcFontSet *font_set;
		font_set = FcFontSort (0, pattern, (ops == OP_ALL) ? FcFalse : FcTrue, 0, &font_result);
		if (!font_set || font_set->nfont == 0) {
			printf("Font not found\n");
			return NULL;
		}
		int j;
		for (j = 0; j < font_set->nfont; j++)
		{
			FcPattern  *font_pattern;

			font_pattern = FcFontRenderPrepare (NULL, pattern, font_set->fonts[j]);
			if (font_pattern)
				FcFontSetAdd (fs, font_pattern);
		}
		FcFontSetDestroy(font_set);
	}
	else{
		//best one font
		FcPattern   *match;
		FcResult result;
		match = FcFontMatch(0, pattern, &result);
		if(match){
			FcFontSetAdd(fs, match);
		}
	}

	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);

	if (!format)
	{
		if (os){
			format = (const FcChar8 *) "%{=unparse}\n";
		}
		else{
			format = (const FcChar8 *) "%{=fcmatch}\n";
		}
	}

	//common code
	char **stringList = NULL;
	int stringCount = 0;

	if (fs)
	{
		int	j;
		for (j = 0; j < fs->nfont; j++)
		{
			FcPattern *font;
			font = FcPatternFilter (fs->fonts[j], os);
			FcChar8 *s;
			s = FcPatternFormat (font, format);
			if(s){
				stringList = realloc(stringList, (stringCount + 1) * sizeof(char *));
				stringList[stringCount] = strdup((const char *)s);
				stringCount++;
				FcStrFree(s);
			}
			
			FcPatternDestroy (font);
		}
		FcFontSetDestroy (fs);
	}

	if(os){
		FcObjectSetDestroy(os);
	}
	FcFini();
	stringList = realloc(stringList, (stringCount + 1) * sizeof(char *));
	stringList[stringCount] = NULL;
	return stringList;
}


int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or unicode or hex\n");
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	setlocale(LC_ALL, "");
	char *input_char = NULL;
	char **mystringList = NULL;
	char **mystringListCopy = NULL;
	
	int ops = OP_NONE;

	struct option longopts[] = {
        {"all", no_argument, NULL, 'a'},
        {"sort", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

	int opt;
	while((opt = getopt_long(argc,argv, "ash", longopts, NULL)) != -1){
		switch (opt)
		{
		case 'a':
			ops = OP_ALL;
			printf("-a argument is there\n");
			break;
		case 's':
			ops = OP_SORT;
			printf("-s argument is there\n");
			break;
		case 'h':
			ops = OP_HELP;
			break;
		default:
			printf("invalid option argument is there\n");
			return 1;
		}
	}
	
	if (ops == OP_HELP) {
		printf("Usage: whichfont [OPTIONS] [INPUT] [FONTNAME] [OTHER PARAMETERS]\n");
		printf("[Input]:\n");
		printf("  1. UTF-8 character  2. UTF-8 hex code  3. Unicode\n");
		printf("[Options]:\n");
		printf("  -a	--all		display all the available matches for the specified font attribute(s)\n");
		printf("  -s	--sort		display sorted list of matches\n");
		printf("  -h	--help		display this help and exit\n");
		printf("[FontName]: \n\tIt can be anything like 'serif', 'monospace', by default 'sans-serif' if no font-name is given\n");
		printf("If you wanna give other parameters then follow this page:\n");
		printf("https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html\n");
		printf("Best of luck using the 'whichfont' tool!\n");
		return 0;
	}

	int k_optind;
	k_optind = optind;
	if(argc == k_optind){
		printf("input character or unicode or hex is needed\n");
		return 1;
	}
	input_char = argv[k_optind];

	int len_inputchar = strlen(input_char);

	int has_digit = 0;
	int has_letter = 0;
	if(len_inputchar > 1){
		for (int i = 0; input_char[i] != '\0'; i++) {
			if (isdigit(input_char[i])) {
				has_digit = 1;
			} else if (isalpha(input_char[i])) {
				has_letter = 1;
			}
		}
	}

	bool hexBool =  len_inputchar >= 2 && (input_char[0] == '0' && (input_char[1] == 'x' || input_char[1] == 'X'));
	bool unicodeBool = len_inputchar >= 2 && input_char[1] == '+' && (input_char[0] == 'U' || input_char[0] == 'u');
	long int unicodepoint;
	if ((hexBool || unicodeBool) || (has_digit==1 && has_letter==0)){
		if(hexBool || unicodeBool){
			input_char += 2;
			int len_input = strlen(input_char);

			if (len_input == 0) { //when input_char=0x or input_char=U+ then it should return false
				printf("empty input argument\n");
				return 0;
			}

			if (hexBool)
			{
				//input is hexadecimal
				if(len_input > 8){
					printf("invalid hexadecimal value\n");
					return 0;
				}
				for (int i = 0; i < len_input; i++) {
					if (!isxdigit(input_char[i])) {
						printf("invalid hexadecimal value\n");
						return 0;
					}
				}
			}
			
		}
	}	
	else
	{
		if(ops == OP_ALL || ops == OP_SORT){
			wchar_t wc;
			char* p = input_char;
			while (*p) {
				int count = mbtowc(&wc, p, MB_CUR_MAX);
				if (count < 0) {
					fprintf(stderr, "Error: invalid multibyte sequence\n");
					return 1;
				} else if (count == 0) {
					fprintf(stderr, "Error: unexpected end of string\n");
					return 1;
				}
				char **mystringList = whichfont((unsigned int) wc, argv, k_optind, ops);
				printf("\n");
				printf("Character: %lc\n", wc);
				int m = 0;
				while (mystringList[m]!=NULL) {
					printf("%s", mystringList[m]);
					free(mystringList[m]);
					m++;
				}
				free(mystringList);
				p += count;
			}
			return 0;
		}
		wchar_t wc;
		wchar_t* wcList = (wchar_t*) malloc(100 * sizeof(wchar_t));
		int wcCount = 0;
		char* p = input_char;
		while (*p) {
			int count = mbtowc(&wc, p, MB_CUR_MAX);
			if (count < 0) {
				fprintf(stderr, "Error: invalid multibyte sequence\n");
				return 1;
			} else if (count == 0) {
				fprintf(stderr, "Error: unexpected end of string\n");
				return 1;
			}
			mystringList = whichfont((unsigned int) wc, argv, k_optind, ops);
			
			if (mystringListCopy)
			{
				int areEqual = 1; // Assume they are equal
				for (int i = 0; mystringList[i] != NULL || mystringListCopy[i] != NULL; i++) {
					if (strcmp(mystringList[i], mystringListCopy[i]) != 0) {
						areEqual = 0; // Not equal
						break;
					}
				}
				if(!areEqual){
					printf("Character: ");
					for (int i = 0; i < wcCount; i++) {
        				printf("%lc", wcList[i]);
    				}
					printf("\n");
					int m = 0;
					while (mystringListCopy[m]) {
						printf("%s", mystringListCopy[m]);
						free(mystringListCopy[m]);
						m++;
					}
					free(mystringListCopy[m]);
					free(wcList);
					wcList = NULL;
					wcCount = 0;
					wcList = (wchar_t*) malloc(100 * sizeof(wchar_t));
					int stringCount = 0;
					mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
					while (mystringList[stringCount]) {
						mystringListCopy[stringCount] = strdup(mystringList[stringCount]);
						stringCount++;
					}
					mystringListCopy[stringCount] = NULL;
				}
			}
			else{
				int stringCount = 0;
				while (mystringList[stringCount]) {
					mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
					mystringListCopy[stringCount] = strdup(mystringList[stringCount]);
					stringCount++;
				}
				mystringListCopy[stringCount] = NULL;
			}
			wcList[wcCount++] = wc;
			p += count;
		}
		printf("Character: ");
		for (int i = 0; i < wcCount; i++) {
			printf("%lc", wcList[i]);
		}
		printf("\n");
		int m = 0;
		while (mystringListCopy[m]) {
			printf("%s", mystringListCopy[m]);
			free(mystringListCopy[m]);
			m++;
		}
		free(mystringList);
		free(mystringListCopy);
		free(wcList);
		return 0;
	}

	char *endptr;
	unicodepoint = strtol(input_char, &endptr, 16);
	if (endptr == input_char || *endptr != '\0' || unicodepoint < 0 || unicodepoint > (hexBool ? 0x7FFFFFFF : 0x10FFFF)) {
		printf("invalid unicodepoint\n");
		return 1;
	}
	mystringList = whichfont(unicodepoint, argv, k_optind, ops);
	int n = 0;
	while (mystringList[n]) {
		printf("%s", mystringList[n]);
		free(mystringList[n]);
		n++;
	}
	free(mystringList);
	return 0;
}
