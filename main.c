#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<ctype.h>
#include<glib.h>
#include<wchar.h>
#include<locale.h>

void matchFontForUTF8_ALL(char* unicode_result) {
	//with -a
	FcCharSet *charset = FcCharSetCreate();
	FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcConfig *config = FcInitLoadConfigAndFonts();
	FcPattern *pattern = FcPatternCreate();
	FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
	if (!pattern) {
        printf("error parsing pattern\n");
        return;
    }
	//FcPatternAddBool(pattern, FC_SCALABLE, FcTrue); //avoiding Fixed, Biwidth fonts(unscalable fonts)
	FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, FC_CHARSET, NULL);
	FcFontSet *font_set = FcFontList(config, pattern, object_set);
	if (font_set == NULL) {
		printf("Font not found\n");
		return;
	}
	for (int i = 0; i < font_set->nfont; i++) {
		FcChar8 *file, *family, *style;
        FcPatternGetString(font_set->fonts[i], FC_FILE, 0, &file);
        FcPatternGetString(font_set->fonts[i], FC_FAMILY, 0, &family);
        FcPatternGetString(font_set->fonts[i], FC_STYLE, 0, &style);
        printf("%s:%s:%s\n", file, family, style);
	}
	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);
	FcObjectSetDestroy(object_set);
	FcFontSetDestroy(font_set);
	FcConfigDestroy(config);
}

void matchFontForUTF8_SORT(char* unicode_result) {
	//with -s
	FcCharSet *charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pattern = FcPatternCreate();
    FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
    if (!pattern) {
        printf("error parsing pattern\n");
        return;
    }
    //FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
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
		    printf ("%s", s);
		    FcStrFree (s);
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

void matchFontForUTF8(char* unicode_result, int argc, char* argv[], int defaultFamily) {
    //selecting the best font
	FcCharSet* charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pattern = FcPatternCreate();
    FcPatternAddCharSet(pattern, FC_CHARSET, charset);
    if (!pattern) {
        printf("error parsing pattern\n");
        return;
    }
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    if (defaultFamily == 1) {
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) argv[2]);
    } else {
        FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*) "sans-serif");
    }
    FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
    FcResult result;
    FcPattern* font = FcFontMatch(NULL, pattern, &result);
    if (font == NULL) {
        printf("Font not found\n");
        return;
    }
    FcChar8* font_path;
    if (FcPatternGetString(font, FC_FILE, 0, &font_path) != FcResultMatch) {
        printf("Font file path not found\n");
        return;
    }
    FcChar8* family;
    int weight, slant;
    FcPatternGetString(font, FC_FAMILY, 0, &family);
    FcPatternGetInteger(font, FC_WEIGHT, 0, &weight);
    FcPatternGetInteger(font, FC_SLANT, 0, &slant);
    printf("Font found: %s: \"%s\" \"%s\"\n", font_path, family,
				weight == FC_WEIGHT_REGULAR && slant == FC_SLANT_ROMAN ? "Regular" : "");
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
			return 1;
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
			printf("enter utf8_character or unicode or hexadecimal");
			return 1;
		}
		else{
			printf("too many arguments");
			return 1;
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
			return 1;
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

	if(len_inputchar >= 2 && (input_char[0] == '0' && (input_char[1] == 'x' || input_char[1] == 'X'))){
		//hexadecimal to unicode
		input_char += 2;
		int len_input = strlen(input_char);
		if (len_input == 0) { //when only 0x is there then it should return false
			printf("empty input argument\n");
        	return 0;
    	}
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
		long long int val = strtoll(input_char, NULL, 16);
    	if (val < 0 || val > 0x7FFFFFFF) {
        	printf("invalid hexadecimal value\n");
			return 0;
    	}
		printf("its hex code with 0xXXXX\n");
		printf("unicode_result: %s\n", input_char);
	}
	else if (len_inputchar >= 2 && input_char[1] == '+' && (input_char[0] == 'U' || input_char[0] == 'u'))
	{
		char *endptr;
		long int codepoint = strtol(input_char + 2, &endptr, 16);
		if (endptr == input_char + 2 || *endptr != '\0' || codepoint < 0 || codepoint > 0x10FFFF) {
			printf("%s is invalid Unicode code point\n", input_char);
			return 1;
		}
		else{
			printf("its uni code with U+XXXX\n");
			input_char += 2;
			printf("unicode_result: %s\n", input_char);
		}
	}
	else if (has_digit==1 && has_letter==0)
	{
		printf("its uni code without having U+, with only digits\n");
		printf("unicode_result: %s\n",input_char);
	}
	else
	{
		//utf8 caharacter to unicode
		printf("its utf8 character\n");
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
        	printf("Character: %lc, Unicode: %s\n", wc, unicode_result);
			if(option == 0){
				matchFontForUTF8(unicode_result, argc, argv, defaultFamily);
			}
			else if(option == 1){
				matchFontForUTF8_ALL(unicode_result);
			}
			else{
				matchFontForUTF8_SORT(unicode_result);
			}
			p += count;
		}
		free(unicode_result);
		return 0;
	}

	//now important function
	if(option == 0){
		matchFontForUTF8(input_char, argc, argv, defaultFamily);
	}
	else if(option == 1){
		matchFontForUTF8_ALL(input_char);
	}
	else{
		matchFontForUTF8_SORT(input_char);
	}
	return 0;
}
