# matchFontForUtf8
## Small tool to query a font for certain code point

At times, we might need to determine the font used to render a particular code point. Although we can use the `fc-match :charset=...` command to get this information, it works only for Unicode. Therefore, to use it with UTF-8 character codes, we first need to convert them to Unicode.

Here I'm presenting a new project that aims to help you determine the font used to render a particular code point. One of the unique features of this tool is its ability to accept input in any format, including `UTF-8 characters`, `Unicode`, or `hexadecimal code points`. This makes it a versatile tool for users who need to work with different types of character encodings.

## User Guidelines:
1. To get started, To get started, you need to use Meson and Ninja to build the project. Meson is a build system that generates build files for Ninja, which is a build tool that builds the project. To build the project. simply run:
    `meson builddir`
    `ninja -C builddir`
    If this is your first time building the project, the first command will create a builddir directory where the build files will be stored. Subsequent builds can skip the meson builddir command.

2. Once you have built the project, you can use the tool to input the code point you want to query. The output will show you the font/fonts used to render that code point. To use the tool, run:
    `./builddir/matchFontForUTF8 कें`

3. Input can be in any format, including UTF-8 characters, Unicode, or hexadecimal code points. Examples include:
    `कें`, `u+0985`, `U+0985`, `U+0985sudiphex`, `U+sudip`, `0x0985`, `0X0985`, `0x0985sudiphex`, `0xsudip`, `0985`, `অah😀`, `A`, `😀`
 
4. Also you can use options like `-a` and `-s` like: `./builddir/matchFontForUTF8 -a कें` or `./builddir/matchFontForUTF8 -s অah😀`
    `-a`: This option specifies that fc-match should return all fonts that match the given pattern, sorted in order of decreasing priority. This can be useful if you want to see all the possible fonts that could be used to render a given character.

    `-s`: This option specifies that fc-match should return only the name of the font that best matches the given pattern. This is the default behavior of fc-match, but it can be useful to explicitly specify the -s option if you want to make sure that only the best match is returned.

## NOTE: 
    Additionally, there is a testcases.sh script file that includes all possible inputs and commands for the tool. If you run `./testcases.sh`, all font results will be stored in `test_results.txt`. For the best viewing experience, you can open test_results.txt with a text editor like gedit.

### Whether you're a developer or a designer, this tool can help you save time and ensure consistency in your work. So give it a try and see how it can help you in your projects!
