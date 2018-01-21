#!/bin/bash

# creates a header file that will be prepended to the hex files created
echo "v2.0 raw" > header.tmp

# runs mars on an assembly file called code.asm
# outputs the text segment in text-out.hex
java -jar mars.jar a dump .text HexText text_t.hex nc code.asm
cat header.tmp text_t.hex > text-out.hex

# runs mars on an assembly file called code.asm
# outputs the data segment in data-out.hex
java -jar mars.jar a dump .data HexText data_t.hex nc code.asm
cat header.tmp data_t.hex > data-out.hex

rm text_t.hex
rm data_t.hex
rm header.tmp
