#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <ctype.h>


char* utf8ToUnicode(char* utf8_char){
	printf("utf8ToUnicode function started\n");
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
    	printf("unicode_result: %s\n", unicode_result);
		return unicode_result;
    }
	if ((utf8[0] & 0xE0) == 0xC0 && len >= 2) {
		unicode = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
    	printf("unicode_result: %s\n", unicode_result);
		return unicode_result;
    }
	if ((utf8[0] & 0xF0) == 0xE0 && len >= 3) {
        unicode = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
    	printf("unicode_result: %s\n", unicode_result);
		return unicode_result;
    }
	if ((utf8[0] & 0xF8) == 0xF0 && len >= 4) {
        unicode = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
		printf("%d utf8tounicode function returns unicode: %d\n",__LINE__, unicode);
		sprintf(unicode_result, "%04x", unicode);
    	printf("unicode_result: %s\n", unicode_result);
		return unicode_result;
    }
	free(unicode_result);
	return NULL;
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("%d Need argument UTF-8 character or hex along with %s\n", __LINE__, argv[0]);
		printf("%d no argument is given\nexiting program...\n",__LINE__);
		return 1;
	}
	
	char *input_char = argv[1];
	printf("%d %s 1st argument\n",__LINE__, input_char);

	//check the input is utf8 or hex or unicode
	char checkchar[20];
	for(int i=0; i< strlen(input_char); i++){
		if(input_char[i] == 'x'){
			printf("%d its hex code with 0xXXXX\n",__LINE__);
			strcpy(checkchar,"hexcode");
			break;
		}
		else if(input_char[i] == 'U'){
			printf("%d its uni code with U+XXXX\n",__LINE__);
			strcpy(checkchar,"unicode");
			break;
		}
		else{
			strcpy(checkchar,"utf8char");
		}
	}

	int has_digit = 0;
    int has_letter = 0;

    for (int i = 0; input_char[i] != '\0'; i++) {
        if (isdigit(input_char[i])) {
            has_digit = 1;
        } else if (isalpha(input_char[i])) {
            has_letter = 1;
        }
    }
    if (has_digit && has_letter) {
		printf("%d its uni code without U+XXXX, with digits and letters\n",__LINE__);
		strcpy(checkchar,"unicodeNoU+");
	}
	if (has_digit || has_letter) {
		printf("%d its uni code without U+XXXX, with digits and letters\n",__LINE__);
		strcpy(checkchar,"unicodeNoU+");
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
	if(strcmp(checkchar,"hexcode")==0){
		//hex with 0xXXXX to unicode
		memmove(input_char, input_char+2, strlen(input_char)+1);  //remove first two characters
		printf("%d unicode: %s\n",__LINE__, input_char);
		unicode_result = input_char;
	}
	if(strcmp(checkchar,"unicode")==0){
		//unicode with U+XXXX to unicode without U+
		memmove(input_char, input_char+2, strlen(input_char)+1);  //remove first two characters
		printf("%d unicode: %s\n",__LINE__, input_char);
		unicode_result = input_char;
	}
	if(strcmp(checkchar,"unicodeNoU+")==0){
		//unicode has (digits and letters both and nothing else) OR (only digits are there)
		printf("%d unicode: %s\n",__LINE__, input_char);
		unicode_result = input_char;
	}

	printf("%d unicode_result: %s\n", __LINE__, unicode_result);


	// Creating FcCharSet from Unicode output above
    FcCharSet *charset = FcCharSetCreate();
    FcCharSetAddChar(charset, (FcChar32) strtol(unicode_result, NULL, 16));

    // Creating FcPattern from FcCharset
    FcConfig* config = FcInitLoadConfigAndFonts();
	FcPattern *pattern = FcPatternCreate();
    FcPatternAddCharSet(pattern, FC_CHARSET, charset); //add charset to font pattern
	if(!pattern){
		printf("%d error parsing pattern\n", __LINE__);
		return 1;
	}
	FcConfigSubstitute(config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);
	
    // Setting the font family if given otherwise sans-serif would be default
	if(argc > 2){
		FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)argv[2]);//adding argument that is given other than sans-serif
	}
	else{
		FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)"sans-serif");//add string value to font pattern
	}
	
	FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);//making sure scalable is true to avoid Fixed, PCF fonts

	// Matching the font
    FcResult result;
    FcPattern *font = FcFontMatch(NULL, pattern, &result);
    if (font == NULL) {
        printf("%d Font not found\n", __LINE__);
        return 1;
    }

	// Getting the font file path
	FcChar8 *font_path;
	if (FcPatternGetString(font, FC_FILE, 0, &font_path) != FcResultMatch) {
		printf("%d Font file path not found\n", __LINE__);
		return 1;
	}
	
	// Getting the font family, weight and slant
	FcChar8 *family;
	int weight, slant;
	FcPatternGetString(font, FC_FAMILY, 0, &family);
	FcPatternGetInteger(font, FC_WEIGHT, 0, &weight);
	FcPatternGetInteger(font, FC_SLANT, 0, &slant);

	// Printing the font information and setting style "Regular" if weight == FC_WEIGHT_REGULAR and slant == FC_SLANT_ROMAN
	printf("%d Font found: %s: \"%s\" \"%s\"\n", __LINE__, font_path, family, 
		weight == FC_WEIGHT_REGULAR && slant == FC_SLANT_ROMAN ? "Regular" : "");

	// memory to be free
	FcCharSetDestroy(charset);
	FcPatternDestroy(pattern);
	FcPatternDestroy(font);
	//free(unicode_result);
	
	return 0;
}
