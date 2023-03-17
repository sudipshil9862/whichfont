#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <ctype.h>

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

char* hex_to_unicode(char* hexcode){
	int unicode = (int)strtol(hexcode, NULL, 16);
	char* unicode_str = (char*)malloc(sizeof(char*)*8);
	//sprintf(unicode_str,"U+%04X",unicode);
	sprintf(unicode_str,"%04X",unicode);
	return unicode_str; 
}

void matchFontForUTF8(char* unicode_result, int argc, char* argv[]) {
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
    if (argc > 2) {
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


int main(int argc, char *argv[]){
	if (argc < 2){
		printf("%d Need argument UTF-8 character or hex along with %s\n", __LINE__, argv[0]);
		printf("%d no argument is given\nexiting program...\n",__LINE__);
		return 1;
	}
	char *input_char = argv[1];

	//check the input is utf8 or hex or unicode
	char checkchar[20] = {'\0'};
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
		printf("%d its hex code with 0xXXXX\n",__LINE__);
		strcpy(checkchar, "hexcode");
	}
	else if (len_inputchar >= 2 && input_char[0] == 'U' && input_char[1] == '+')
	{
		printf("%d its uni code with U+XXXX\n",__LINE__);
		strcpy(checkchar, "unicode");
	}
	else if (has_digit==1 && has_letter==0 && checkchar[0] == '\0')
	{
		printf("%d its uni code without having U+, with only digits\n",__LINE__);
		strcpy(checkchar,"unicodeNoU+");
	}
	else if (has_digit==1 && has_letter==1 && checkchar[0] == '\0')
	{
		printf("%d its uni code without having U+, with only digits and letters\n",__LINE__);
		strcpy(checkchar,"unicodeNoU+");
	}
	else{
		strcpy(checkchar, "utf8char");
	}

	char *unicode_result = (char*) malloc(sizeof(char) * 8);

	if(strcmp(checkchar,"utf8char")==0){
		//utf8 caharacter to unicode
		printf("%d its utf8 character\n",__LINE__);
		unicode_result = utf8ToUnicode(input_char);
		if(unicode_result){
			printf("%d unicode_result: %s\n", __LINE__,unicode_result);
		}
	}
	else if(strcmp(checkchar,"hexcode")==0){
		//hexadecimal to unicode
		unicode_result = hex_to_unicode(input_char);
		printf("Decimal value: %d\n", (int) strtol(input_char, NULL, 16));
		printf("%d unicode_result: %s\n",__LINE__, unicode_result);
	}
	else if(strcmp(checkchar,"unicode")==0){
		//unicode with U+XXXX to unicode without U+
		memmove(input_char, input_char+2, strlen(input_char)+1);  //remove first two characters
		printf("%d unicode_result: %s\n",__LINE__, input_char);
		unicode_result = input_char;
	}
	else if(strcmp(checkchar,"unicodeNoU+")==0){
		unicode_result = hex_to_unicode(input_char);
		printf("Decimal value: %d\n", (int) strtol(input_char, NULL, 16));
		printf("%d unicode_result: %s\n",__LINE__, unicode_result);
	}
	else{
		printf("%d unknown input format", __LINE__);
	}
	
	matchFontForUTF8(unicode_result, argc, argv);

	return 0;
}
