#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<ctype.h>
#include<glib.h>
#include<wchar.h>

char* utf8ToUnicode(char* utf8_char){
	int len = strlen(utf8_char);
	unsigned char *utf8 = (unsigned char*)utf8_char; //utf8 is a pointer to the input string that is cast to an unsigned char pointer
	//I'm ensuring that byte values are interpreted correctly
	int unicode;
	char *unicode_result = (char*) malloc(sizeof(char) * 8);
	if(len==0){
		return NULL;
	}
	if (utf8[0] <= 0x7F) {
		unicode = utf8[0];
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
		return unicode_result;
	}
	if ((utf8[0] & 0xE0) == 0xC0 && len >= 2) {
		unicode = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
		return unicode_result;
	}
	if ((utf8[0] & 0xF0) == 0xE0 && len >= 3) {
		unicode = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
		return unicode_result;
	}
	if ((utf8[0] & 0xF8) == 0xF0 && len >= 4) {
		unicode = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
		return unicode_result;
	}
	free(unicode_result);
	return NULL;
}

void matchFontForUTF8_ALL(char* unicode_result) {
	//with -a
	FcCharSet *charset = FcCharSetCreate();
	FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));
	FcConfig *config = FcInitLoadConfigAndFonts();
	FcPattern *pattern = FcPatternCreate();
	FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
	if (!pattern) {
        printf("%d error parsing pattern\n", __LINE__);
        return;
    }
	//FcPatternAddBool(pattern, FC_SCALABLE, FcTrue); //avoiding Fixed, Biwidth fonts(unscalable fonts)
	FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, FC_CHARSET, NULL);
	FcFontSet *font_set = FcFontList(config, pattern, object_set);
	if (font_set == NULL) {
		printf("%d Font not found\n", __LINE__);
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
        printf("%d error parsing pattern\n", __LINE__);
        return;
    }
    //FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);
	FcResult font_result; //error handling if any so we need this font_result
    FcObjectSet *object_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_INDEX, NULL);
	FcFontSet *font_set = FcFontSort (0, pattern, FcTrue, 0, &font_result);
	if (font_set == NULL || font_set->nfont == 0) {
        printf("%d Font not found\n", __LINE__);
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
        printf("%d error parsing pattern\n", __LINE__);
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
        printf("%d Font not found\n", __LINE__);
        return;
    }
    FcChar8* font_path;
    if (FcPatternGetString(font, FC_FILE, 0, &font_path) != FcResultMatch) {
        printf("%d Font file path not found\n", __LINE__);
        return;
    }
    FcChar8* family;
    int weight, slant;
    FcPatternGetString(font, FC_FAMILY, 0, &family);
    FcPatternGetInteger(font, FC_WEIGHT, 0, &weight);
    FcPatternGetInteger(font, FC_SLANT, 0, &slant);
    printf("%d Font found: %s: \"%s\" \"%s\"\n", __LINE__, font_path, family,
				weight == FC_WEIGHT_REGULAR && slant == FC_SLANT_ROMAN ? "Regular" : "");
	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);
	FcPatternDestroy(font);
}

int is_valid_hex(char* input) {
    int len = strlen(input);
    if (len < 3 || input[0] != '0' || tolower(input[1]) != 'x') {
        return 0;
    }
    for (int i = 2; i < len; i++) {
        if (!isxdigit(input[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("%d Need argument UTF-8 character or hex along with %s\n", __LINE__, argv[0]);
		printf("%d no argument is given\nexiting program...\n",__LINE__);
		return 1;
	}
	char *input_char = NULL;
	int defaultFamily = 0; //sans-serif
	int option = 0; //if -a,-s not there
	if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "-s") == 0))
	{
		if(strcmp(argv[1], "-a")==0){
			printf("%d -a argument is there\n",__LINE__);
			option = 1; // 1 for -a
		}
		else if (strcmp(argv[1], "-s")==0)
		{
			printf("%d -s argument is there\n",__LINE__);
			option = 2; //2 for -s
		}
		else
		{
			printf("%d invalid option argument is there\n",__LINE__);
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

	char *unicode_result = (char*) malloc(sizeof(char) * 8);

	if(len_inputchar >= 2 && (input_char[0] == '0' && (input_char[1] == 'x' || input_char[1] == 'X'))){
		//hexadecimal to unicode
		if (is_valid_hex(input_char)) {
			printf("%d its hex code with 0xXXXX\n",__LINE__);
			input_char += 2;
			printf("%d unicode_result: %s\n",__LINE__, unicode_result);
			unicode_result = input_char;
		}
		else{
			printf("invalid hexadecimal value\n");
			return 1;
		}
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
			printf("%d its uni code with U+XXXX\n",__LINE__);
			input_char += 2;
			printf("%d unicode_result: %s\n",__LINE__, input_char);
			unicode_result = input_char;
		}
	}
	else if (has_digit==1 && has_letter==0)
	{
		printf("%d its uni code without having U+, with only digits\n",__LINE__);
		unicode_result = input_char;
		printf("%d unicode_result: %s\n",__LINE__, unicode_result);
	}
	else
	{
		//utf8 caharacter to unicode
		printf("%d its utf8 character\n",__LINE__);		
		gchar* p = input_char;
		while (*p != '\0') {
			gunichar unicode = g_utf8_get_char(p);
			gchar utf8_str[5];
			g_unichar_to_utf8(unicode, utf8_str);
			printf("%d unicode_result of %s: %04x\n",__LINE__, utf8_str, unicode);
			sprintf(unicode_result, "%04x", unicode);
			p = g_utf8_next_char(p);
			if(unicode_result){
				printf("%d unicode_result: %s\n", __LINE__,unicode_result);
			}
			if(option == 0){
				matchFontForUTF8(unicode_result, argc, argv, defaultFamily);
			}
			else if(option == 1){
				matchFontForUTF8_ALL(unicode_result);
			}
			else{
				matchFontForUTF8_SORT(unicode_result);
			}
		}
		return 1;
	}

	//now important function
	if(option == 0){
		matchFontForUTF8(unicode_result, argc, argv, defaultFamily);
	}
	else if(option == 1){
		matchFontForUTF8_ALL(unicode_result);
	}
	else{
		matchFontForUTF8_SORT(unicode_result);
	}
	return 0;
}
