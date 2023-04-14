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
		OP_END
};

void whichfont(long int unicodepoint, char* argv[], int k_optind, int ops){
	FcPattern *pattern;
	FcCharSet *charset;
	FcObjectSet	*os = 0;
	const FcChar8 *format = NULL;
	
	
	pattern = FcPatternCreate();
	charset = FcCharSetCreate();
	
	FcCharSetAddChar(charset, (FcChar32) unicodepoint);
	FcPatternAddCharSet(pattern, FC_CHARSET, charset);

	if (!pattern)
	{
		printf ("Unable to parse the pattern\n");
		return;
	}
	while (argv[++k_optind])
	{
		if (!os)
		{
			os = FcObjectSetCreate ();
		}
		FcObjectSetAdd (os, argv[k_optind]);
	}

	if(!pattern){
		printf ("Unable to parse the pattern\n");
		return;
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
			return;
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
				printf("%s", s);
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
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or hex along with %s\n", argv[0]);
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	setlocale(LC_ALL, "");
	char *input_char = NULL;
	
	int ops = OP_NONE;

	int opt;
	while((opt = getopt(argc,argv, "as")) != -1){
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
		default:
			printf("invalid option argument is there\n");
			return 1;
		}
	}

	int k_optind;
	k_optind = optind;
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
		wchar_t wc;
		char* unicode_result = (char*) malloc(5 * sizeof(char));
		char *endptr;
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
			sprintf(unicode_result, "%04X", (unsigned int) wc);
			printf("\n");
			printf("Character: %lc\n", wc);
			printf("\n");
			whichfont(strtol(unicode_result, &endptr, 16), argv, k_optind, ops);
			p += count;
		}
		free(unicode_result);
		return 0;
	}

	char *endptr;
	unicodepoint = strtol(input_char, &endptr, 16);
	if (endptr == input_char || *endptr != '\0' || unicodepoint < 0 || unicodepoint > (hexBool ? 0x7FFFFFFF : 0x10FFFF)) {
		printf("invalid unicodepoint\n");
		return 1;
	}
	whichfont(unicodepoint, argv, k_optind, ops);
	return 0;
}
