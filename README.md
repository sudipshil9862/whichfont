# whichfont
## Querying Fontconfig

Introducing a new project that simplifies the process of determining the font used to render a specific code point. Unlike the `fc-match :charset=... command`, which only works for `Unicode`, this tool can handle input in various formats, such as `UTF-8 characters`, `Unicode`. This versatility makes it a valuable tool for users who work with different character encodings.

### Note for Fedora users: please use `whichfont` instead of using `./.builddir/whichfont` or `./run.sh`. 
### Only github users will use `./.builddir/whichfont` or `./run.sh`

## User Guidelines:
1. To get started, To get started, you need to use Meson and Ninja to build the project. Meson is a build system that generates build files for Ninja, which is a build tool that builds the project. To build the project. simply run:
    ```
    ./build.sh
    ```
    or run manually using below commands:
    ```
    meson setup .builddir
    ```
    ```
    ninja -C .builddir
    ```
    If this is your first time building the project, the first command will create a builddir directory where the build files will be stored. Subsequent builds can skip the meson builddir command.

2. Once you have built the project, you can use the tool to input the code point you want to query. The output will show you the font/fonts used to render that code point. To use the tool, run:
    ```
    ./.builddir/whichfont <your_input>
    ```
3. The easy and best way is to run `run.sh` script like: `./run.sh`. This will build and run simultaneously like:
    ```
    ./run.sh <your_input>
    ```

3. Input can be in any format, including UTF-8 characters, Unicode. Examples include:
    `कें`, `u+0985`, `U+0985`, `u+0985`, `U+sudip`, `0x0985`, `0X0985`, `0xsudip`, `0985`, `অah😀`, `RED`, `A`, `😀`
 
4. Also you can use options like `-a` and `-s` like: ./.builddir/whichfont [option] <your_input> [option]
    ```
    ./run.sh -a कें
    ``` 
    or 
    ```
    ./run.sh -s অah😀
    ```
    `-a`: This option specifies that fc-match should return all fonts that match the given pattern, sorted in order of decreasing priority. This can be useful if you want to see all the possible fonts that could be used to render a given character.

    `-s`: This option specifies that fc-match should return all the best sorted list of fonts match for a given font pattern or font name.

5. Another option can be [font name]:
   FontName can be anything like 'serif', 'monospace', by default 'sans-serif' if no font-name is given.
   ```
   ./run.sh -f "FONTNAME" <your_input>
   ```
   example: 
   ```   
   ./run.sh -f "Noto Sans Bengali" 0985 
   ```

6. For instant help use: `-h` or `--help`
    ```
     ./run.sh --help
    ```

## NOTE: 

Additionally, there is a testcases.sh script file that includes all possible inputs and commands for the tool.
run:
```
./tests.sh
```
all font results will be stored in `test_results.txt`. For the best viewing experience, you can open `test_results.txt` with a text editor like gedit.

#### Whether you're a developer or a designer, this tool can help you save time and ensure consistency in your work. So give it a try and see how it can help you in your projects!
