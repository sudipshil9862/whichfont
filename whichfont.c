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


void whichfont_ALL(char* unicode_result, char* argv[], int defaultFamily) {
	//with -a
	FcPattern *pattern = FcPatternCreate();
	FcCharSet *charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
    FcConfig *config = FcInitLoadConfigAndFonts();
	if (defaultFamily == 1) {
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) argv[2]);
    }
	FcResult font_result; //error handling if any so we need this font_result
    FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, NULL);
	FcFontSet *font_set = FcFontSort (0, pattern, FcFalse, 0, &font_result);
	if (font_set == NULL || font_set->nfont == 0) {
        printf("Font not found\n");
        return;
    }
 	FcFontSet *fs;
	fs = FcFontSetCreate ();
	int j;
	for (j = 0; j < font_set->nfont; j++)
	{
	    FcPattern  *font_pattern;

	    font_pattern = FcFontRenderPrepare (NULL, pattern, font_set->fonts[j]);
	    if (font_pattern)
			FcFontSetAdd (fs, font_pattern);
	}
	if (fs)
    {
	int	j;
	for (j = 0; j < fs->nfont; j++)
	{
	    FcPattern *font;
		FcObjectSet	*os = 0;
	    font = FcPatternFilter (fs->fonts[j], os);
	    FcChar8 *s;
		const FcChar8 *format = NULL;
		format = (const FcChar8 *) "%{=fcmatch}\n";
		s = FcPatternFormat (font, format);
		if(s)
		{
			FcChar8 *family, *style;
			FcPatternGetString(fs->fonts[j], FC_FAMILY, 0, &family);
			FcPatternGetString(fs->fonts[j], FC_STYLE, 0, &style);
			printf("\"%s\" \"%s\"\n", family, style);
		}
	    FcPatternDestroy (font);
	}
	FcFontSetDestroy (fs);
    }
    FcCharSetDestroy(charset);
    FcPatternDestroy(pattern);
    FcObjectSetDestroy(object_set);
    FcFontSetDestroy(font_set);
    FcConfigDestroy(config);
}

void whichfont_SORT(char* unicode_result, char* argv[], int defaultFamily) {
	//with -s
	FcPattern *pattern = FcPatternCreate();
	FcCharSet *charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
    FcConfig *config = FcInitLoadConfigAndFonts();
	if (defaultFamily == 1) {
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) argv[2]);
    }
	FcResult font_result; //error handling if any so we need this font_result
    FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, NULL);
	FcFontSet *font_set = FcFontSort (0, pattern, FcTrue, 0, &font_result);
	if (font_set == NULL || font_set->nfont == 0) {
        printf("Font not found\n");
        return;
    }
 	FcFontSet *fs;
	fs = FcFontSetCreate ();
	int j;
	for (j = 0; j < font_set->nfont; j++)
	{
	    FcPattern  *font_pattern;

	    font_pattern = FcFontRenderPrepare (NULL, pattern, font_set->fonts[j]);
	    if (font_pattern)
			FcFontSetAdd (fs, font_pattern);
	}
	if (fs)
    {
	int	j;
	for (j = 0; j < fs->nfont; j++)
	{
	    FcPattern *font;
		FcObjectSet	*os = 0;
	    font = FcPatternFilter (fs->fonts[j], os);
	    FcChar8 *s;
		const FcChar8 *format = NULL;
		format = (const FcChar8 *) "%{=fcmatch}\n";
		s = FcPatternFormat (font, format);
		if(s)
		{
			FcChar8 *family, *style;
			FcPatternGetString(fs->fonts[j], FC_FAMILY, 0, &family);
			FcPatternGetString(fs->fonts[j], FC_STYLE, 0, &style);
			printf("\"%s\" \"%s\"\n", family, style);
		}
	    FcPatternDestroy (font);
	}
	FcFontSetDestroy (fs);
    }
    FcCharSetDestroy(charset);
    FcPatternDestroy(pattern);
    FcObjectSetDestroy(object_set);
    FcFontSetDestroy(font_set);
    FcConfigDestroy(config);
}

void whichfont(char* unicode_result, char* argv[], int defaultFamily) {
    //selecting the best font
	FcPattern* pattern = FcPatternCreate();
	FcCharSet* charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcPatternAddCharSet(pattern, FC_CHARSET, charset);
    FcConfig* config = FcInitLoadConfigAndFonts();
	if (defaultFamily == 1) {
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) argv[2]);
    }
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern* font = FcFontMatch(NULL, pattern, &result);
    if (font == NULL) {
        printf("Font not found\n");
        return;
    }
	FcChar8 *family, *style;
    FcPatternGetString(font, FC_FAMILY, 0, &family);
    FcPatternGetString(font, FC_STYLE, 0, &style);
    printf("\"%s\" \"%s\"\n", family, style);
	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);
	FcPatternDestroy(font);
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or hex along with %s\n", argv[0]);
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	char *input_char = NULL;
	int defaultFamily = 0; //sans-serif
	int option = 0; //if -a,-s not there
	if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "-s") == 0))
	{
		if(strcmp(argv[1], "-a")==0){
			printf("-a argument is there\n");
			option = 1; // 1 for -a
		}
		else if (strcmp(argv[1], "-s")==0)
		{
			printf("-s argument is there\n");
			option = 2; //2 for -s
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
		/*
		printf("length of input_char %d\n", len_input);
		char* input_copy = strdup(input_char + 2);
		int len_copy = strlen(input_copy);
		printf("length of input_copy %d\n", len_copy);
		*/
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
			//printf("its hex code with 0xXXXX\n");
			//printf("unicode_result: %s\n", input_char);
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
			/*
			else
			{
				printf("its uni code with U+XXXX\n");
				printf("unicode_result: %s\n", input_char);
			}
			*/
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
		/*
		else{
			printf("its unicode digits\n");
			printf("unicode_result: %s\n", input_char);
		}
		*/
	}
	else
	{
		//input is utf8 caharacter
		//printf("its utf8 character\n");
		setlocale(LC_ALL, "");
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
        	//printf("Character: %lc, Unicode: %s\n", wc, unicode_result);
			printf("Character: %lc\n", wc);
			if(option == 0){
				whichfont(unicode_result, argv, defaultFamily);
			}
			else if(option == 1){
				whichfont_ALL(unicode_result, argv, defaultFamily);
			}
			else{
				whichfont_SORT(unicode_result, argv, defaultFamily);
			}
			p += count;
		}
		free(unicode_result);
		return 0;
	}

	if(option == 0){
		whichfont(input_char, argv, defaultFamily);
	}
	else if(option == 1){
		whichfont_ALL(input_char, argv, defaultFamily);
	}
	else{
		whichfont_SORT(input_char, argv, defaultFamily);
	}
	return 0;
}
