# png_steno

Hide plain text messages into PNG images!

## Build

This repo pulls in a required library as a git submodule. Make sure to clone
the submodule as well.

After downloading, you can build with a simple

```sh
make
```

## Run

To encode a message stored in `message.txt` into an image `vessel.png` and
output the resulting PNG to `secret.png`, you can run:

```sh
./encoder vessel.png secret.png message.txt
```

The message file should be an actual file, not a pipe or process substitution.
We have to be able to `rewind(3)` it.

To decode a message stored in `secret.png`, you can run:

```sh
./decoder secret.png
```

The message will then be printed to `stdout`.

## Restrictions

This program only supports encoding messages that fall under the printable ASCII
range, along with line feed (`\n`), horizontal tab (`\t`), and carriage return
(`\r`). The message must not contain any null (`\0`) characters, or things will
go horribly wrong. Basically, use chars from this dictionary:

```c
"\t\n\r!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
```

This program tries its best to compress your data, but it must limit the length
of your message to however much can be stored in the image you provide. If this
limit is exceeded, a message informing you will be printed to `stderr`.

## Design

### Overview

The basic flow of encoding looks like this:

```
[Encode message] -> [Add error detection redundancy] -> [Squirrel away into PNG]
```

Decoding looks ~the same but in reverse:

```
[Extract from PNG] -> [Decode message] -> [Check for errors] -> [Print message]
```

We'll go into more detail about the encode, error detection, and data storage
steps below.

### Data storage

How might we store data into an image? Begin by noting that each pixel in a PNG
(well, most PNGs) contain four channels: red, green, blue, and alpha. These
channels are (usually) stored as a single byte. Changing the two least
significant bits in each channel has a negligible effect on the channel's visual
appearance. This sounds promising. What if we could change these two least
significant bits for each pixel across some portion of an image to store our
data? Conveniently enough, two bits from each of the 4 channels makes a byte.
This program exploits these observations to store arbitrary messages into the
RGBA channels of an image.

### Message encoding

I chose to use Huffman coding to compress provided message data for two reasons:
(1) it is a provably optimal character-based encoding scheme and (2) I already
kind of knew how to do it from my Algorithms class. To add some extra challenge,
I decided to use a specific flavor of Huffman called a [canonical Huffman
code](https://en.wikipedia.org/wiki/Canonical_Huffman_code). Doing so lets us
encode our token dictionary in only 38 bytes. This is actually the same scheme
used by the DEFLATE compression algorithm.

Encoding with my Huffman implementation cuts the size of large messages roughly
in half.

### Error detection

Our error detection approach is a bit simpler. We simply take a CRC32 of the
encoded Huffman data and append it to the end of what we write back to the PNG.
When decoding, we read this stored CRC32 and compare it to a CRC32 we calculate
on received data. I chose CRC32 because it was simple to implement and provides
a reasonably strong defense against errors in transmission. I researched both
Hamming and Reed-Solomon codes for a while, but ultimately didn't feel
comfortable enough with them conceptually to hit the road coding them out.
Eventually I'd like to come back to this and get R-S working, it seems super
cool.

### PNG Library

This project uses
[`stb_image`](https://github.com/nothings/stb/blob/master/stb_image.h) for
image encoding and [Wuffs the
Library](https://github.com/google/wuffs/blob/main/doc/wuffs-the-library.md)
(with its `stb_image` compatibility layer) for image decoding. These libraries
are extremely easy to use--they're each implemented in one header file and the
exposed API is dead simple. This choice was made under the assumption that we
cannot trust our input PNGs, but don't need a hyper-optimized output image. If
we wanted to hyper-optimize our output image we might run it through
[oxipng](https://github.com/shssoichiro/oxipng) after encoding, or use a more
sophisticated encoder. All image-interaction code is abstracted into its own
tiny API in `image.h`, so swapping between libraries should be pretty easy.
I've been happy using `stb_image` and Wuffs so far.
