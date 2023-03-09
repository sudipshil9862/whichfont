# matchFontForUtf8
## Small tool to query a font for certain code point

We sometimes want to see what a font is used to render a certain code point.  Although we can ask for that with 
`fc-match :charset=...`
,this is in Unicode.  We need to convert UTF-8 character code to Unicode to do this.
