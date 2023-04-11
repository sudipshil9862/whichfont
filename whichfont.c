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

void whichfont(char* unicode_result, char* argv[], int defaultFamily, int all, int sort){
	FcPattern *pattern = FcPatternCreate();
	FcCharSet *charset = FcCharSetCreate();
	FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
	
	if (defaultFamily == 1) {
		FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) argv[2]);
	}

	FcFontSet *fs;
	fs = FcFontSetCreate ();

	FcConfigSubstitute (0, pattern, FcMatchPattern);
    FcDefaultSubstitute (pattern);

	if(all || sort){
		//with -a or -s
		FcResult font_result; //error handling if any so we need this font_result
		FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, NULL);
		FcFontSet *font_set;
		if(all){
			font_set = FcFontSort (0, pattern, FcFalse, 0, &font_result);
		}
		else if (sort)
		{
			font_set = FcFontSort (0, pattern, FcTrue, 0, &font_result);
		}
		if (font_set == NULL || font_set->nfont == 0) {
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
		FcObjectSetDestroy(object_set);
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

	//common code
	if (fs)
	{
		int	j;
		for (j = 0; j < fs->nfont; j++)
		{
			FcPattern *font;
			FcObjectSet	*os = 0;
			font = FcPatternFilter (fs->fonts[j], os);
			FcChar8 *family, *style;
			FcPatternGetString(fs->fonts[j], FC_FAMILY, 0, &family);
			FcPatternGetString(fs->fonts[j], FC_STYLE, 0, &style);
			printf("\"%s\" \"%s\"\n", family, style);
			FcPatternDestroy (font);
		}
		FcFontSetDestroy (fs);
	}
	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or hex along with %s\n", argv[0]);
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	setlocale(LC_ALL, "");
	char *input_char = NULL;
	int defaultFamily = 0; //sans-serif
	int all = 0; //-a
	int sort = 0; //-s
	if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "-s") == 0))
	{
		if(strcmp(argv[1], "-a")==0){
			printf("-a argument is there\n");
			all = 1;
		}
		else if (strcmp(argv[1], "-s")==0)
		{
			printf("-s argument is there\n");
			sort = 1;
		}
		else
		{
			printf("invalid option argument is there\n");
			return 0;
		}
		//checking other arguments
		if (argc == 3)
		{
			input_char = argv[2];
		}
		else if(argc == 4){
			input_char = argv[2];
			defaultFamily = 1; //some other family
		}
		else if (argc == 2)
		{
			printf("enter utf8_character or unicode or hexadecimal\n");
			return 0;
		}
		else{
			printf("too many arguments\n");
			return 0;
		}
	}
	else{
		if (argc == 3)
		{
			input_char = argv[1];
			defaultFamily = 1; //some other family
		}
		else if (argc == 2)
		{
			input_char = argv[1];
		}
		else{
			printf("too many arguments");
			return 0;
		}
	}

	int len_inputchar = strlen(input_char);

	int has_digit = 0;
	int has_letter = 0;
	for (int i = 0; input_char[i] != '\0'; i++) {
		if (isdigit(input_char[i])) {
			has_digit = 1;
		} else if (isalpha(input_char[i])) {
			has_letter = 1;
		}
	}

	bool hexBool =  len_inputchar >= 2 && (input_char[0] == '0' && (input_char[1] == 'x' || input_char[1] == 'X'));
	bool unicodeBool = len_inputchar >= 2 && input_char[1] == '+' && (input_char[0] == 'U' || input_char[0] == 'u');

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
			long int val = strtol(input_char, NULL, 16);
			if (val < 0 || val > 0x7FFFFFFF) {
				printf("invalid hexadecimal value\n");
				return 0;
			}
		}
		else if (unicodeBool)
		{
			// input is unicode
			char *endptr;
			long int codepoint = strtol(input_char, &endptr, 16);
			if (endptr == input_char || *endptr != '\0' || codepoint < 0 || codepoint > 0x10FFFF)
			{
				printf("%s is invalid Unicode code point\n", input_char);
				return 0;
			}
		}
	}
	else if (has_digit==1 && has_letter==0)
	{
		//input is unicode
		char *endptr;
		long int codepoint = strtol(input_char, &endptr, 16);
		if (endptr == input_char || *endptr != '\0' || codepoint < 0 || codepoint > 0x10FFFF) {
			printf("%s is invalid Unicode code point\n", input_char);
			return 1;
		}
	}
	else
	{
		wchar_t wc;
		char* unicode_result = (char*) malloc(5 * sizeof(char));
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
			whichfont(unicode_result, argv, defaultFamily, all, sort);
			p += count;
		}
		free(unicode_result);
		return 0;
	}
	whichfont(input_char, argv, defaultFamily, all, sort);
	return 0;
}
