GOES LRIT Channel Decoder
=========================

This program decodes a demuxed channel data. To run:

```
  python channeldecoder.py ID
```

This will look for `channels/channel_ID.bin` and create the **LRIT** files inside `channels/ID/`. If enabled in header, the script will also call **wine** and run **decompressor.exe** to decompress the packets `(Enabled by Default)`

For the full usage make sure you have **decompressor.exe**, **packetmanager.py** in the same folder as **channeldecoder.py** and **wine** installed. 

With all **LRIT** files, you can use [xrit2pic](http://www.alblas.demon.nl/wsat/software/soft_msg.html) to show the files and get images like this:

![GOES 13 Fulldisk](fulldisk.jpg)