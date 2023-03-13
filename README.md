# matchFontForUtf8
## Small tool to query a font for certain code point

We sometimes want to see what a font is used to render a certain code point.  Although we can ask for that with 
`fc-match :charset=...`
,this is in Unicode.  We need to convert UTF-8 character code to Unicode to do this.

## User Guidelines:
1.   First time user run below commands in your terminal:
        `meson builddir`</br>
        `ninja -C builddir`</br>
    Second time user can skip `meson builddir`
2. Input can be anything: utf8_char, hex, unicode
    Here is the example:
        `./builddir/matchFontForUTF8 कें`
        `./builddir/matchFontForUTF8 😀`
        `./builddir/matchFontForUTF8 0041`
        `./builddir/matchFontForUTF8 099E`
        `./builddir/matchFontForUTF8 REDHAT`
### Try it out
