# ChordSpell
### A spell checker for music chords
Please note that i'm a noob when it comes to music theory. Parts of the dictionaries i generated and provide might be wrong. Please let me know if you find errors in the dictionaries.

## Build

Make sure you have g++ 5 or higher and type 'make'

## Usage

Extract the dictionaries (or create your own) and run chord_spell on an input file (line separated chords).

Run 'src/spell/chord_spell --help' for more options.

## Example

    $> echo "Am#" | chord_spell /dev/stdin
    A#m

