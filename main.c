#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>

char* convertToHex(char *utf8_char){
	printf("%d function to convert utf8 char to hexcode\n",__LINE__);
	int i,len;
	char* utf8;
	len = strlen(utf8_char);
	// each byte is represented by two hex digits, plus one for the null terminator
	utf8 = (char*)malloc(sizeof(char) * (len * 2 + 1));
	for(i = 0; i < len; i++){
		// each hex digit is represented by two characters, plus one for the null terminator
		char* hex = (char*)malloc(sizeof(char) * 3);
		// convert the byte to a two-digit hex string
		sprintf(hex, "%02x", (unsigned char)utf8_char[i]);
		// append the hex string to the output string
		strcat(utf8, hex);
		free(hex);
	}
	return utf8;
}


char* hexToUnicode(char* s){
	int len = strlen(s)/2;
	unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned char) * len);
	int i, j, numBytes;
	unsigned int codePoint;
	char* unicode = (char*)malloc(sizeof(char) * (len + 1));
	// Convert hexadecimal string to byte array
	for (i = 0; i < len; i++){
		sscanf(s + i*2, "%2hhx", &bytes[i]);
	}
	// Convert byte array to Unicode code points
	i=0;
	j=0;
	while (i < len) {
		if (bytes[i] < 0x80) {
			codePoint = bytes[i];
			numBytes = 1;
		} else if (bytes[i] < 0xE0) {
			codePoint = ((bytes[i] & 0x1F) << 6) | (bytes[i+1] & 0x3F);
			numBytes = 2;
		} else if (bytes[i] < 0xF0) {
			codePoint = ((bytes[i] & 0x0F) << 12) | ((bytes[i+1] & 0x3F) << 6) | (bytes[i+2] & 0x3F);
			numBytes = 3;
		} else {
			codePoint = ((bytes[i] & 0x07) << 18) | ((bytes[i+1] & 0x3F) << 12) | ((bytes[i+2] & 0x3F) << 6) | (bytes[i+3] & 0x3F);
			numBytes = 4;
		}
		unicode[j++] = (char)codePoint;
		i += numBytes;
	}
	unicode[j] = '\0';
	free(bytes);
	return unicode;
}

int utf8ToUnicode(char* utf8_char){
	printf("utf8ToUnicode function started\n");
	int len = strlen(utf8_char);
	unsigned char *utf8 = (unsigned char*)utf8_char; //utf8 is a pointer to the input string that is cast to an unsigned char pointer (to ensure that byte values are interpreted correctly)
	int unicode;
	/*if(len==0){
		return -1;
	}*/
	if (utf8[0] <= 0x7F) {
		unicode = utf8[0];
		return unicode;
    }
	if ((utf8[0] & 0xE0) == 0xC0 && len >= 2) {
		unicode = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
		return unicode;
    }
	if ((utf8[0] & 0xF0) == 0xE0 && len >= 3) {
        unicode = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
		return unicode;
    }
	if ((utf8[0] & 0xF8) == 0xF0 && len >= 4) {
        unicode = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
		return unicode;
    }
	return -1;
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
	int unicode;
	if(strcmp(checkchar,"utf8char")==0){
		printf("%d its utf8 character\n",__LINE__);
		/*char* hex = convertToHex(input_char);
		printf("%d hex: %s\n",__LINE__, hex);*/
		//int unicode;
		unicode = utf8ToUnicode(input_char);
		printf("%d Unicode: %04X\n", __LINE__, unicode);
	}
	if(strcmp(checkchar,"hexcode")==0){
		//hex to unicode
		memmove(input_char, input_char+2, strlen(input_char)+1);  //remove first two characters
		printf("%d unicode: %s\n",__LINE__, input_char);
		//unicode = input_char;
		//char* hex_to_unicode = hexToUnicode(input_char);
		//printf("hex to unicode: %s\n",hex_to_unicode);
	}
	if(strcmp(checkchar,"unicode")==0){
		memmove(input_char, input_char+2, strlen(input_char)+1);  //remove first two characters
		printf("%d unicode: %s\n",__LINE__, input_char);
		//unicode = input_char;
	}
	
	return 0;
}
