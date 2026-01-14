#include<fontconfig/fontconfig.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include<wchar.h>
#include<wctype.h>
#include<locale.h>
#include<unistd.h>
#include<getopt.h>

enum {
		OP_NONE = 0,
		OP_ALL,
		OP_SORT,
		OP_FONTFAMILY,
		OP_LANGUAGE,
		OP_LIST_LANGUAGES,
		OP_HELP,
		OP_END
};

const char *valid_langs[] = {
	"aa", "ab", "af", "agr", "ak", "am", "an", "anp", "ar", "as",
	"ast", "av", "ay", "ayc", "az_az", "az_ir", "ba", "be", "bem", "ber_dz",
	"ber_ma", "bg", "bh", "bhb", "bho", "bi", "bin", "bm", "bn", "bo",
	"br", "brx", "bs", "bua", "byn", "ca", "ce", "ch", "chm", "chr",
	"ckb", "cmn", "co", "cop", "crh", "cs", "csb", "cu", "cv", "cy",
	"da", "de", "doi", "dsb", "dv", "dz", "ee", "el", "en", "eo",
	"es", "et", "eu", "fa", "fat", "ff", "fi", "fil", "fj", "fo",
	"fr", "fur", "fy", "ga", "gd", "gez", "gl", "gn", "got", "gu",
	"gv", "ha", "hak", "haw", "he", "hi", "hif", "hne", "ho", "hr",
	"hsb", "ht", "hu", "hy", "hz", "ia", "id", "ie", "ig", "ii",
	"ik", "io", "is", "it", "iu", "ja", "jv", "ka", "kaa", "kab",
	"ki", "kj", "kk", "kl", "km", "kn", "ko", "kok", "kr", "ks",
	"ku_am", "ku_iq", "ku_ir", "ku_tr", "kum", "kv", "kw", "kwm", "ky", "la",
	"lah", "lb", "lez", "lg", "li", "lij", "ln", "lo", "lt", "lv",
	"lzh", "mag", "mai", "mfe", "mg", "mh", "mhr", "mi", "miq", "mjw",
	"mk", "ml", "mn_cn", "mn_mn", "mni", "mnw", "mo", "mr", "ms", "mt",
	"my", "na", "nan", "nb", "nds", "ne", "ng", "nhn", "niu", "nl",
	"nn", "no", "nqo", "nr", "nso", "nv", "ny", "oc", "om", "or",
	"os", "ota", "pa", "pa_pk", "pap_an", "pap_aw", "pes", "pl", "prs", "ps_af",
	"ps_pk", "pt", "qu", "quz", "raj", "rif", "rm", "rn", "ro", "ru",
	"rw", "sa", "sah", "sat", "sc", "sco", "sd", "se", "sel", "sg",
	"sgs", "sh", "shn", "shs", "si", "sid", "sk", "sl", "sm", "sma",
	"smj", "smn", "sms", "sn", "so", "sq", "sr", "ss", "st", "su",
	"sv", "sw", "syr", "szl", "ta", "tcy", "te", "tg", "th", "the",
	"ti_er", "ti_et", "tig", "tk", "tl", "tn", "to", "tpi", "tr", "ts",
	"tt", "tw", "ty", "tyv", "ug", "uk", "und_zmth", "und_zsye", "unm", "ur",
	"uz", "ve", "vi", "vo", "vot", "wa", "wae", "wal", "wen", "wo",
	"xh", "yap", "yi", "yo", "yue", "yuw", "za", "zh_cn", "zh_hk", "zh_mo",
	"zh_sg", "zh_tw", "zu",
	NULL
};


char** whichfont(long int unicodepoint, long int vs_codepoint, char* argv[], int p_index, int ops, const char* fontfamily){
	FcPattern *pattern;
	FcCharSet *charset;
	FcObjectSet *os = 0;
	const FcChar8 *format = NULL;
	
	
	pattern = FcPatternCreate();
	if (!pattern)
	{
		printf ("Unable to parse the pattern\n");
		return NULL;
	}
	charset = FcCharSetCreate();
	
	FcCharSetAddChar(charset, (FcChar32) unicodepoint);
	if (vs_codepoint != 0){
		FcCharSetAddChar(charset, (FcChar32) vs_codepoint);
	}
	FcPatternAddCharSet(pattern, FC_CHARSET, charset);

	//printf("p_index=%d\n", p_index);
	if(p_index != -1) {
		while (argv[++p_index])
		{
			if (!os)
			{
				os = FcObjectSetCreate ();
			}
			FcObjectSetAdd (os, argv[p_index]);
		}
	}
	
	if (ops == OP_FONTFAMILY && fontfamily) {
        	FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)fontfamily);
    	}
    	else{
    		FcPatternAddString(pattern, FC_FAMILY, (FcChar8*)"sans-serif");
    	}

	if (vs_codepoint == 0xFE0F || unicodepoint >= 0x1F000) {
		FcPatternAddBool(pattern, FC_COLOR, FcTrue);	
	}

	FcConfigSubstitute (0, pattern, FcMatchPattern);
    	FcDefaultSubstitute (pattern);

	FcFontSet *fs;
	fs = FcFontSetCreate ();

	if(ops==OP_ALL || ops==OP_SORT){
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
			//filter out fonts that don't actually support the requested char.
			//FcFontSort returns a prioritized fontlist that contain the glyph including fallbacks.
			FcCharSet *font_cs;
			if (FcPatternGetCharSet(font_set->fonts[j], FC_CHARSET, 0, &font_cs) == FcResultMatch) {
				if (!FcCharSetHasChar(font_cs, (FcChar32)unicodepoint)) {
					continue;
				}
				if (vs_codepoint == 0xFE0F) {
					FcPatternAddBool(pattern, FC_COLOR, FcTrue);	
				}

			}
			
			FcPattern  *font_pattern;
			font_pattern = FcFontRenderPrepare (NULL, pattern, font_set->fonts[j]);
			if (font_pattern)
				FcFontSetAdd (fs, font_pattern);
		}
		FcFontSetDestroy(font_set);
	}
	else{//for -f and if no option
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

char* wcharToString(int charvalue){

        /* This is a **complete** list of all non-printable characters */
	const char *charMapping[] = {
                [0x0000] = "NULL",
                [0x0001] = "START OF HEADING",
                [0x0002] = "START OF TEXT",
                [0x0003] = "END OF TEXT",
                [0x0004] = "END OF TRANSMISSION",
                [0x0005] = "ENQUIRY",
                [0x0006] = "ACKNOWLEDGE",
                [0x0007] = "BELL",
                [0x0008] = "BACKSPACE",
                [0x0009] = "CHARACTER TABULATION",
                [0x000A] = "LINE FEED (LF)",
                [0x000B] = "LINE TABULATION",
                [0x000C] = "FORM FEED (FF)",
                [0x000D] = "CARRIAGE RETURN (CR)",
                [0x000E] = "SHIFT OUT",
                [0x000F] = "SHIFT IN",
                [0x0010] = "DATA LINK ESCAPE",
                [0x0011] = "DEVICE CONTROL ONE",
                [0x0012] = "DEVICE CONTROL TWO",
                [0x0013] = "DEVICE CONTROL THREE",
                [0x0014] = "DEVICE CONTROL FOUR",
                [0x0015] = "NEGATIVE ACKNOWLEDGE",
                [0x0016] = "SYNCHRONOUS IDLE",
                [0x0017] = "END OF TRANSMISSION BLOCK",
                [0x0018] = "CANCEL",
                [0x0019] = "END OF MEDIUM",
                [0x001A] = "SUBSTITUTE",
                [0x001B] = "ESCAPE",
                [0x001C] = "INFORMATION SEPARATOR FOUR",
                [0x001D] = "INFORMATION SEPARATOR THREE",
                [0x001E] = "INFORMATION SEPARATOR TWO",
                [0x001F] = "INFORMATION SEPARATOR ONE",
                [0x007F] = "DELETE",
                [0x0080] = "U+0080 control character",
                [0x0081] = "U+0081 control character",
                [0x0082] = "BREAK PERMITTED HERE",
                [0x0083] = "NO BREAK HERE",
                [0x0084] = "U+0084 control character",
                [0x0085] = "NEXT LINE (NEL)",
                [0x0086] = "START OF SELECTED AREA",
                [0x0087] = "END OF SELECTED AREA",
                [0x0088] = "CHARACTER TABULATION SET",
                [0x0089] = "CHARACTER TABULATION WITH JUSTIFICATION",
                [0x008A] = "LINE TABULATION SET",
                [0x008B] = "PARTIAL LINE FORWARD",
                [0x008C] = "PARTIAL LINE BACKWARD",
                [0x008D] = "REVERSE LINE FEED",
                [0x008E] = "SINGLE SHIFT TWO",
                [0x008F] = "SINGLE SHIFT THREE",
                [0x0090] = "DEVICE CONTROL STRING",
                [0x0091] = "PRIVATE USE ONE",
                [0x0092] = "PRIVATE USE TWO",
                [0x0093] = "SET TRANSMIT STATE",
                [0x0094] = "CANCEL CHARACTER",
                [0x0095] = "MESSAGE WAITING",
                [0x0096] = "START OF GUARDED AREA",
                [0x0097] = "END OF GUARDED AREA",
                [0x0098] = "START OF STRING",
                [0x0099] = "U+0099 control character",
                [0x009A] = "SINGLE CHARACTER INTRODUCER",
                [0x009B] = "CONTROL SEQUENCE INTRODUCER",
                [0x009C] = "STRING TERMINATOR",
                [0x009D] = "OPERATING SYSTEM COMMAND",
                [0x009E] = "PRIVACY MESSAGE",
                [0x009F] = "APPLICATION PROGRAM COMMAND",
                [0x2028] = "LINE SEPARATOR",
                [0x2029] = "PARAGRAPH SEPARATOR",
	};
        if(charvalue > 0x2029) {
            /* Avoid segfault if accessing after the end of the
             * array. wcharToString should never be called for indices
             * outside of this array, but just in case, if called by
             * accident for invalid indices, don’t segfault. */
            return "This should never happen!";
        }
	if(charMapping[charvalue] != NULL){
                return strdup(charMapping[charvalue]);
	}
	else{
		return strdup("non-printable character");
	}
	
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or unicode\n");
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	
	setlocale(LC_ALL, "");
	char *input_char;
	input_char = malloc(1024);  // or dynamically calculate the needed size
	if (!input_char) {
	    fprintf(stderr, "Memory allocation failed for input_char\n");
	    exit(EXIT_FAILURE);
	}
	input_char[0] = '\0';  // Initialize as empty string before strcat
			       //
	char **mystringList = NULL;
	char **mystringListCopy = NULL;
	char *fontfamily = NULL;
	
	int ops = OP_NONE;

	struct option longopts[] = {
        	{"all", no_argument, NULL, 'a'},
        	{"sort", no_argument, NULL, 's'},
        	{"font", no_argument, NULL, 'f'},
		{"language", required_argument, NULL, 'l'},
		{"list-languages", no_argument, NULL, 'L'},
        	{"help", no_argument, NULL, 'h'},
        	{NULL, 0, NULL, 0}
    	};
    	
	

	int opt;
	opterr = 0;
	int p_index = -1;
	int param_mode = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "::") == 0 || strcmp(argv[i], "--") == 0) {
			param_mode = 1;
			p_index = i;
			break;
		}
	}
	int opt_argc = (p_index == -1) ? argc : p_index;
	while ((opt = getopt_long(opt_argc, argv, "asful:hL", longopts, NULL)) != -1) {
	    switch (opt) {
		case 'a':
		    if (ops != OP_NONE) {
			printf("-a is not acceptable with other options (-s, -f, -l, -L, -h)\n");
			return 1;
		    }
		    ops = OP_ALL;
		    break;
		case 's':
		    if (ops != OP_NONE) {
			printf("-s is not acceptable with other options (-a, -f, -l, -L, -h)\n");
			return 1;
		    }
		    ops = OP_SORT;
		    break;
		case 'f':
		    if (ops != OP_NONE) {
			printf("-f is not acceptable with other options (-a, -s, -l, -L, -h)\n");
			return 1;
		    }
		    ops = OP_FONTFAMILY;
		    break;
		case 'l':
		    if (ops != OP_NONE) {
			printf("-l is not acceptable with other options (-a, -s, -f, -L, -h)\n");
			return 1;
		    }
		    if (optarg == NULL) {
			printf("missing argument for -l\n");
			return 1;
		    }
		    ops = OP_LANGUAGE;
		    fontfamily = strdup(optarg);
		    break;
		case 'L':
		    if (ops != OP_NONE) {
			printf("-L is not acceptable with other options (-a, -s, -f, -l, -h)\n");
			return 1;
		    }
		    ops = OP_LIST_LANGUAGES;
		    break;
		case 'h':
		    if (ops != OP_NONE) {
			printf("-h is not acceptable with other options (-a, -s, -f, -l, -L)\n");
			return 1;
		    }
		    ops = OP_HELP;
		    break;
		case '?':
		    if (optopt == 'l') {
			printf("missing argument for -l\n");
		    }
		    else {
		    	printf("invalid option argument is there\n");
		    }
		    return 1;
	    }
	}
	
	if (ops == OP_HELP) {
		printf("Usage: whichfont [OPTION] <INPUT> [:: PARAMS]\n");
		printf("       whichfont -f 'Noto Sans' abc def :: family style weight\n");
		printf("       whichfont abc def :: family familylang\n");
		printf("       whichfont -a abc def :: family familylang\n");
		printf("       whichfont --language ja\n");
		printf("<Input>:\n");
		printf("  1. UTF-8 character or Unicode\n");
		printf("[Options]:\n");
		printf("  -a	--all		display all the available matches for the specified font attribute(s)\n");
		printf("  -s	--sort		display sorted list of matches\n");
		printf("  -f	--font		specify the fontname\n");
		printf("  -l    --language	detect default font and validate font support for a given language\n");
		printf("  -L    --list-languages   display all supported language codes\n");
		printf("  -h	--help		display this help and exit\n");
		printf("<PARAMS>:\nIf you wanna give other parameters then follow this page:\n");
		printf("https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html\n");
		printf("\nLearn more about whichfont: https://github.com/sudipshil9862/whichfont/blob/main/README.md\n");
		return 0;
	}

	if (ops == OP_LIST_LANGUAGES) {
		printf("Supported language codes: \n");
		for (int i = 0; valid_langs[i] != NULL; i++) {
			printf("%s%s", valid_langs[i], ((i + 1) % 10 == 0) ? "\n" : " ");
		}
		printf("\n");
		return 0;
	}

	
	
	int k_optind;
	k_optind = optind;
	if (ops == OP_LANGUAGE) {
	    if (fontfamily == NULL) {
			printf("Please provide a language code with --language option\n");
			return 1;
		}
		int is_valid_lang = 0;
		for (int i = 0; valid_langs[i] != NULL; i++) {
			if (strcmp(valid_langs[i], fontfamily) == 0) {
				is_valid_lang = 1;
				break;
			}
		}
		if (!is_valid_lang) {
			printf("Invalid language code used\n");
			printf("Use --list-languages to see the supported list\n");
			return 0;
		}
		FcPattern *pattern = FcPatternCreate();
		FcPatternAddString(pattern, FC_LANG, (FcChar8 *)fontfamily);
		FcConfigSubstitute(NULL, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);
		
		FcResult result;
		FcPattern *match = FcFontMatch(NULL, pattern, &result);
		if (!match) {
			printf("No font installed for (%s) language\n", fontfamily);
			FcPatternDestroy(pattern);
			return 0;
		}
		FcLangSet *available_langs;
		if (FcPatternGetLangSet(match, FC_LANG, 0, &available_langs) == FcResultMatch) {
			FcLangSet *requested_lang = FcLangSetCreate();
			FcLangSetAdd(requested_lang, (const FcChar8 *) fontfamily);
			if (!FcLangSetContains(available_langs, requested_lang)) {
				printf("No font installed for (%s) language\n", fontfamily);
				FcLangSetDestroy(requested_lang);
				FcPatternDestroy(match);
				FcPatternDestroy(pattern);
				return 0;
			}
			FcLangSetDestroy(requested_lang);
		}
		FcChar8 *family = NULL;
		if (FcPatternGetString(match, FC_FAMILY, 0, &family) == FcResultMatch) {
			printf("%s\n", family);
		} else {
			printf("Font found but family name could not be retrieved\n");
		}
		FcPatternDestroy(match);
		FcPatternDestroy(pattern);
		return 0;
	}
	if(k_optind < argc && argv[k_optind] != NULL && strcmp(argv[k_optind], "::") == 0){
		printf("no input characters to render\n");
		return -1;
	}
	if(param_mode){
		if(argv[p_index + 1] == NULL){
			printf("no fontconfig parameters entered after using :: separator\n");
			return 1;	
		}
	}
	
	if(ops == OP_FONTFAMILY){
		if(argc == k_optind){
			printf("input character or unicode is needed\n");
			return 1;
		}
		if(argc == k_optind+1){
			printf("please specify both fontname and unicode\n");
			return 1;
		}
	}
	
	if(ops == OP_FONTFAMILY){
		fontfamily = (char *)malloc(strlen(argv[k_optind]) + 1); // +1 for the null terminator
		if (fontfamily != NULL) {
			strcpy(fontfamily, argv[k_optind]);
		} else {
			// handling memory allocation failure
			fprintf(stderr, "Memory allocation failed for fontfamily.\n");
			exit(EXIT_FAILURE);
		}
		k_optind++;
		if(param_mode){
			for (int i = k_optind; i < p_index; i++) {
				strcat(input_char, argv[i]);
				if (i != p_index - 1){
					strcat(input_char, " ");
				}
			}
		}
		else {
			for(int i = k_optind; i < argc; i++){
				strcat(input_char, argv[i]);
				if(i != argc - 1){
					strcat(input_char, " ");
				}
			}
                }
	}
	else{
		if(param_mode){
                        for (int i = k_optind; i < p_index; i++) {
                                strcat(input_char, argv[i]);
                                if (i != p_index - 1){
                                        strcat(input_char, " ");
                                }
                        }
                }
		else {
			for(int i = k_optind; i < argc; i++){
				strcat(input_char, argv[i]);
				if(i != argc - 1){
					strcat(input_char, " ");
				}
			}
		}
	}

	printf("input_char: %s\n", input_char);

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
		// NOTE: Handling raw unicode values (0x...) doesn't easily support Variation Selectors logic 
		// unless the input format explicitly supports it (e.g. 0x260E 0xFE0F).
		// For now, assuming raw hex input is a single character as per original code.
		char *endptr;
		unicodepoint = strtol(input_char, &endptr, 16);
		if (endptr == input_char || *endptr != '\0' || unicodepoint < 0 || unicodepoint > (hexBool ? 0x7FFFFFFF : 0x10FFFF)) {
			printf("invalid unicodepoint\n");
			return 1;
		}
		// Pass 0 as vs_codepoint for direct hex input
		mystringList = whichfont(unicodepoint, 0, argv, p_index, ops, fontfamily);
		int n = 0;
		while (mystringList[n]) {
			printf("%s", mystringList[n]);
			free(mystringList[n]);
			n++;
		}
		free(mystringList);
		return 0;
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
				
				// Lookahead for Variation Selector (U+FE0F)
				wchar_t next_wc = 0;
				int next_count = 0;
				if (*(p + count)) {
					next_count = mbtowc(&next_wc, p + count, MB_CUR_MAX);
				}
				long int vs_val = 0;
				if (next_wc == 0xFE0F) {
					vs_val = 0xFE0F;
				}
				
				char **mystringList = whichfont((unsigned int) wc, vs_val, argv, p_index, ops, fontfamily);
				printf("\n");
				if (vs_val) {
					printf("'<U+%04X> + <U+%04X>'  ->  \" %lc%lc \"\n", (unsigned int)wc, (unsigned int)vs_val, wc, (wchar_t)vs_val);
					p += (count + next_count);
				}
				else {
					if (!iswprint((wint_t)wc)) {
						char* charString = wcharToString((unsigned int)wc);
						printf("\"%s\" ", charString);
						free(charString);
					}
					else {
						printf("\"%lc\" ", wc);
					}
					printf("<U+%04X>\n", (unsigned int)wc);
					p += count;
				}
			        	
				int m = 0;
				if(mystringList) {
					while (mystringList[m]!=NULL) {
						printf("%s", mystringList[m]);
						free(mystringList[m]);
						m++;
					}
					free(mystringList);
				}
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
			
			// Lookahead for Variation Selector
			wchar_t next_wc = 0;
			int next_count = 0;
			if (*(p + count)) {
				next_count = mbtowc(&next_wc, p + count, MB_CUR_MAX);
			}
			long int vs_val = 0;
			if (next_wc == 0xFE0F) {
				vs_val = 0xFE0F;
			}
			mystringList = whichfont((unsigned int) wc, vs_val, argv, p_index, ops, fontfamily);
			
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
					printf("\n");
					for (int i = 0; i < wcCount; i++) {
						if (i + 1 < wcCount && wcList[i+1] == 0xFE0F) {
							 printf("'<U+%04X> + <U+%04X>'  ->  \" %lc%lc \"\n", 
								   (unsigned int)wcList[i], 
								   (unsigned int)wcList[i+1], 
								   wcList[i], wcList[i+1]);
							 i++;
						} else {
							if (!iswprint((wint_t)wcList[i])) {
								char* charString = wcharToString((unsigned int)wcList[i]);
								printf("\"%s\" ", charString);
								free(charString);
							} else {
								printf("\"%lc\" ", wcList[i]);
							}
							printf("<U+%04X>\n",(unsigned int) wcList[i]);
						}
					}
					int m = 0;
					while (mystringListCopy[m]) {
						printf("%s", mystringListCopy[m]);
						free(mystringListCopy[m]);
						m++;
					}
					free(wcList);
					wcList = (wchar_t*) malloc(100 * sizeof(wchar_t));
					wcCount = 0;
					int stringCount = 0;
					if (mystringListCopy) free(mystringListCopy); // Avoid double free/leak
					mystringListCopy = NULL;
					while (mystringList[stringCount]) {
						mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
						mystringListCopy[stringCount] = strdup(mystringList[stringCount]);
						stringCount++;
					}
					if(mystringListCopy){
						mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
						mystringListCopy[stringCount] = NULL;
					}
				}
			}
			else{
				int stringCount = 0;
				while (mystringList[stringCount]) {
					mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
					mystringListCopy[stringCount] = strdup(mystringList[stringCount]);
					stringCount++;
				}
				if(mystringListCopy){
					mystringListCopy = realloc(mystringListCopy, (stringCount + 1) * sizeof(char *));
					mystringListCopy[stringCount] = NULL;
				}
			}
			wcList[wcCount++] = wc;
			if (vs_val) {
				wcList[wcCount++] = (wchar_t)vs_val;
				p += (count + next_count);
			} else {
				p += count;
			}
			int m = 0;
			while (mystringList[m]) {
				free(mystringList[m]);
				m++;
			}
			free(mystringList);
		}
		printf("\n");
		for (int i = 0; i < wcCount; i++) {
			if (i + 1 < wcCount && wcList[i+1] == 0xFE0F) {
				printf("'<U+%04X> + <U+%04X>'  ->  \" %lc%lc \"\n", 
					(unsigned int)wcList[i], 
					(unsigned int)wcList[i+1], 
					wcList[i], wcList[i+1]);
				i++;
			}
			else {
				if (!iswprint((wint_t)wcList[i])) {
					char* charString = wcharToString((unsigned int)wcList[i]);
					printf("\"%s\" ", charString);
					free(charString);
				} else {
					printf("\"%lc\" ", wcList[i]);
				}
				printf("<U+%04X>\n",(unsigned int) wcList[i]);
			}
		}	
		int m = 0;
		if(mystringListCopy){
			while (mystringListCopy[m]) {
				printf("%s", mystringListCopy[m]);
				free(mystringListCopy[m]);
				m++;
			}
			free(mystringListCopy);
		}
		free(wcList);
		return 0;
	}
	return 0;
}
