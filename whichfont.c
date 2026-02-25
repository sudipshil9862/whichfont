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
#include <glib.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>

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

bool is_likely_emoji(long int cp) {
    // original emoji blocks (SMP - Plane 1)
    // Covers all modern emojis (faces, food, animals, transport)
    if (cp >= 0x1F000 && cp <= 0x1FFFF) return true;
    if (cp >= 0x2600 && cp <= 0x26FF) return true;
    if (cp >= 0x2700 && cp <= 0x27BF) return true;//dinfbats
    if (cp >= 0x2B50 && cp <= 0x2B55) return true; // geomatric shape extended
    if (cp >= 0x2300 && cp <= 0x23FF) return true;
    if (cp >= 0x1F000 && cp <= 0x1F0FF) return true;
    if (cp >= 0x1F100 && cp <= 0x1F1FF) return true;  // enclosed alphanumeric

    return false;
}


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

	//if (vs_codepoint == 0xFE0F || unicodepoint >= 0x1F000) {
	if (vs_codepoint == 0xFE0F || is_likely_emoji(unicodepoint)) {
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
			FcCharSet *font_cs;
			if (FcPatternGetCharSet(font_set->fonts[j], FC_CHARSET, 0, &font_cs) == FcResultMatch) {
				if (!FcCharSetHasChar(font_cs, (FcChar32)unicodepoint)) {
					continue;
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


void print_segment_chars(const char *text, int length) {
    long char_count = g_utf8_strlen(text, length);

    if (char_count > 1) {
        printf("%.*s\n", length, text);
        return;
    }

    wchar_t wc;
    int i = 0;
    while(i < length) {
        int count = mbtowc(&wc, text + i, length - i);
        if (count <= 0) break;

        wchar_t next_wc = 0;
        int next_count = 0;
        if (i + count < length) {
            next_count = mbtowc(&next_wc, text + i + count, length - (i + count));
        }

        if (next_wc == 0xFE0F) {
            printf("'<U+%04X> + <U+%04X>' -> \" %lc%lc \" ", (unsigned int)wc, (unsigned int)next_wc, wc, next_wc);
            i += next_count;
        } else {
             if (!iswprint((wint_t)wc)) {
                char* s = wcharToString((unsigned int)wc);
                printf("\"%s\" ", s);
                free(s);
            } else {
                printf("\"%lc\" ", wc);
            }
            printf("<U+%04X> ", (unsigned int)wc);
        }
        i += count;
    }
    printf("\n");
}


void resolve_via_pango(const char *input_text, const char *fontfamily, const char *lang_code, char* argv[], int p_index) {
    PangoFontMap *fontmap = pango_ft2_font_map_new();
    PangoContext *context = pango_font_map_create_context(fontmap);
    PangoAttrList *attrs = pango_attr_list_new();
    if (fontfamily) {
        PangoAttribute *attr_fam = pango_attr_family_new(fontfamily);
        pango_attr_list_insert(attrs, attr_fam);
    } 
    if (lang_code) {
        PangoLanguage *lang = pango_language_from_string(lang_code);
        PangoAttribute *attr_lang = pango_attr_language_new(lang);
        pango_attr_list_insert(attrs, attr_lang);
        pango_context_set_language(context, lang);
    }
    GList *items = pango_itemize(context, input_text, 0, strlen(input_text), attrs, NULL);
    GList *l;
    for (l = items; l; l = l->next) {
        PangoItem *item = (PangoItem *)l->data;
        const char *segment_text = input_text + item->offset;
        int segment_len = item->length;
        
        print_segment_chars(segment_text, segment_len);

        PangoFont *font = item->analysis.font;
        if (font) {
            PangoFontDescription *desc = pango_font_describe(font);
            char *desc_str = pango_font_description_to_string(desc);
            g_free(desc_str);
            pango_font_description_free(desc);
            if (PANGO_IS_FC_FONT(font)) {
                PangoFcFont *fc_font = PANGO_FC_FONT(font);
                FcPattern *pattern = pango_fc_font_get_pattern(fc_font);
                if (pattern) {
                    FcObjectSet *os = 0;
                    const FcChar8 *format = NULL;
                     if(p_index != -1) {
                        while (argv[++p_index]) {
                            if (!os) os = FcObjectSetCreate ();
                            FcObjectSetAdd (os, argv[p_index]);
                        }
                        p_index -= (p_index - (p_index));
                    }
                    if (!format) {
                        if (os) format = (const FcChar8 *) "%{=unparse}\n";
                        else format = (const FcChar8 *) "%{=fcmatch}\n";
                    }
                    FcPattern *filtered_pat = os ? FcPatternFilter(pattern, os) : FcPatternDuplicate(pattern);
                    FcChar8 *s = FcPatternFormat(filtered_pat, format);
                    if (s) {
                        printf("%s", s);
                        FcStrFree(s);
                    }
                    FcPatternDestroy(filtered_pat);
                    if (os) FcObjectSetDestroy(os);
                }
            }
        } else {
            printf("No font found for this segment.\n");
        }
    }
    g_list_free_full(items, (GDestroyNotify)pango_item_free);
    pango_attr_list_unref(attrs);
    g_object_unref(context);
    g_object_unref(fontmap);
}


int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Need argument UTF-8 character or unicode\n");
		printf("no argument is given\nexiting program...\n");
		return 1;
	}
	
	setlocale(LC_ALL, "");
	char *input_char;
	input_char = malloc(4096);
	if (!input_char) {
	    fprintf(stderr, "Memory allocation failed for input_char\n");
	    exit(EXIT_FAILURE);
	}
	input_char[0] = '\0';  // Initialize as empty string before strcat
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
	if (ops == OP_FONTFAMILY) {
		if(k_optind < argc) {
		     fontfamily = strdup(argv[k_optind]);
		     k_optind++;
		}
	}
	int end_loop = param_mode ? p_index : argc;
	for(int i = k_optind; i < end_loop; i++){
		strcat(input_char, argv[i]);
		if(i != end_loop - 1) strcat(input_char, " ");
	}
	if (strlen(input_char) == 0 && ops != OP_LANGUAGE) {
		printf("No input text provided.\n");
		return 1;
	}
	bool hexBool = (strlen(input_char) >= 2 && input_char[0] == '0' && (input_char[1] == 'x' || input_char[1] == 'X'));
	bool unicodeBool = (strlen(input_char) >= 2 && (input_char[0] == 'U' || input_char[0] == 'u') && input_char[1] == '+');
	
	bool pureHexBool = (strlen(input_char) >= 4);
	if (pureHexBool) {
		for (int i = 0; input_char[i] != '\0'; i++) {
			if (!isxdigit(input_char[i])) {
			    pureHexBool = false;
			    break;
			}
		}
	}	
	
	if (hexBool || unicodeBool || pureHexBool) {
		char *ptr;
		if (hexBool || unicodeBool) {
			ptr = input_char + 2;
		} else {
			ptr = input_char;
		}

		char *endptr;
		long int cp = strtol(ptr, &endptr, 16);

		if (*endptr != '\0') {
			printf("invalid unicodepoint\n");
			return 1;
		}

		if (ptr == endptr) {
			printf("invalid unicodepoint\n");
			return 1;
		}

		if (cp < 0 || cp > 0x10FFFF) {
			printf("Invalid Unicode Code Point: U+%lX (Values must be between 0 and 0x10FFFF)\n", cp);
			return 1;
		}

		if (ops == OP_ALL || ops == OP_SORT) {
			char **res = whichfont(cp, 0, argv, p_index, ops, fontfamily);
			if(res) {
				int n=0; while(res[n]) { printf("%s", res[n]); free(res[n]); n++; }
				free(res);
			}
			return 0;
		}
		
		char utf8_buf[8] = {0};
		if (wctomb(utf8_buf, (wchar_t)cp) == -1) {
			printf("Could not convert U+%lX to UTF-8\n", cp);
			return 1;
		}
		resolve_via_pango(utf8_buf, fontfamily, NULL, argv, p_index);
		return 0;
	}
		
	if (ops == OP_ALL || ops == OP_SORT) {
		wchar_t wc;
		char* p = input_char;
		while (*p) {
		    int count = mbtowc(&wc, p, MB_CUR_MAX);
		    if(count <=0) break;
		    wchar_t next_wc = 0;
		    int next_count = 0;
		    if (*(p+count)) next_count = mbtowc(&next_wc, p+count, MB_CUR_MAX);
		    long int vs = (next_wc == 0xFE0F) ? 0xFE0F : 0;
		    char **res = whichfont((long)wc, vs, argv, p_index, ops, fontfamily);
		    printf("\nChar: %lc (U+%04X)\n", wc, (unsigned int)wc);
		    if(res) {
			int n=0; while(res[n]) { printf("%s", res[n]); free(res[n]); n++; }
			free(res);
		    }
		    p += count;
		    if(vs) p += next_count;
		}
	}
	else {
		char* lang = (ops == OP_LANGUAGE) ? fontfamily : NULL;
		char* fam = (ops == OP_FONTFAMILY) ? fontfamily : NULL;
		resolve_via_pango(input_char, fam, lang, argv, p_index);
	}
	free(input_char);
	if(fontfamily && ops != OP_LANGUAGE) free(fontfamily);
	return 0;	
}
