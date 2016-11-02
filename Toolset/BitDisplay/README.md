BitDisplay
==================

This is a Bit Viewer for binary files. It shows the bits in a graphical format.

Usage

```bash
  BitDisplay filename.bin width [bitmode]
```

If bitmode is 1, it will display each bit on the file as a pixel on the screen. If bitmode is not specified or 0 it will use each byte as a pixel color on the screen (packed as RGB8).