# whichfont

## Querying Fontconfig

Introducing a new project that simplifies the process of determining the font used to render a specific code point. Unlike the `fc-match :charset=...` command, which only works for `Unicode`, this tool can handle input in various formats, such as `UTF-8 characters`, `Unicode`. This versatility makes it a valuable tool for users who work with different character encodings.

### Note for Fedora users: `whichfont` is available in Fedora repositories! 🎉  
You can install it directly using:
```sh
sudo dnf install whichfont
```

Then run it like this:
```sh
whichfont --language ja
```

### If you cloned from GitHub, use `./.builddir/whichfont` or `./run.sh`.

## Installation

To get started, install the dependencies (Meson, Ninja, Fontconfig headers). On Fedora:
```sh
sudo dnf install meson ninja-build fontconfig-devel
```

## Building

Use Meson and Ninja to build the project. Meson generates build files and Ninja builds the project.

### Automatic:
```sh
./build.sh
```

### Manual:
```sh
meson setup .builddir
ninja -C .builddir
```

The first command will create a `.builddir/` directory to store build files. On subsequent builds, you can skip the Meson setup step.

## Usage

Once built, you can use the tool as follows:
```sh
./.builddir/whichfont <INPUT> [:: PARAMS]
```

Or use the convenience script:
```sh
./run.sh <your_input> [:: PARAMS]
```

### Input formats supported
- UTF-8 characters
- Unicode in forms like `U+0985`, `0x0985`, `0985`
- Mixed input like `অah😀`, `RED`, `A`, `abc def`

### Examples:
`./run.sh abc`
`./run.sh -f "Noto Sans" abc def`
`./run.sh -f "Noto Sans" abc def :: family style weight`
`./run.sh abc def :: family familylang`
`./run.sh --language ja`

## Options

### `-a`, `--all`
Show all matching fonts in descending order of priority.
```sh
./run.sh -a कें
```

### `-s`, `--sort`
Return the best sorted list of matching fonts.
```sh
./run.sh -s অah😀
```

### `-f`, `--font`
Specify the font name (defaults to `sans-serif`).
```sh
./run.sh -f "Noto Sans Bengali" 0985
```

### `-l`, `--language`
Check if a valid font is installed for the given language code.

#### Examples:
- Valid language code with font installed:
  ```sh
  ./run.sh --language ja
  ```
  Output: `Noto Sans CJK JP`

- Valid language code but **no font installed**:
  ```sh
  ./run.sh --language ccp
  ```
  Output: `No font installed for (ccp) language`

- Invalid language code:
  ```sh
  ./run.sh --language xyz
  ```
  Output: `Invalid language code used`

This feature checks:
- If the language code is valid (based on a known list)
- If any font matches that language
- If the font truly declares support via `FcLangSetContains()`

### `-L`, `--list-languages`
Display all supported language codes from the internal database.
```sh
./run.sh --list-languages
```

### ::PARAMS
Parameters provided after :: are treated as Fontconfig attributes.
You can specify attributes like family, style, weight, lang, etc.
If you wanna give other parameters then follow this page:
https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html

#### Examples:
`./run.sh -f "Noto Sans" abc def :: family style weight`
`./run.sh abc def :: family weight lang`

### `-h`, `--help`
Display usage instructions.
```sh
./run.sh --help
```

## Tests

Run all test cases:
```sh
./tests.sh
```

Results are saved in `test_results.txt` for review.

---

Whether you're a developer or a designer, this tool can help you save time and ensure consistency in your work. Give it a try and see how it can simplify your font management tasks!
