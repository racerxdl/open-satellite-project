#!/usr/bin/env python

'''
    GOES LRIT Packet Manager
    Copyright (C) 2016 Lucas Teske <lucas {at} teske {dot] net [dot} br>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

'''

import os, struct

'''
  File Type Codes:
  0 - Image
  2 - Text
  130 - DCS Data
'''

def manageFile(filename):
  f = open(filename, "r")

  try:
    k = readHeader(f)
    type, filetypecode, headerlength, datalength = k
  except:
    print "   Header 0 is corrupted for file %s" %filename
    return

  newfilename = filename
  while f.tell() < headerlength:
    data = readHeader(f)
    if data[0] == 4:
      #print "   Filename is %s" % data[1]
      newfilename = data[1]
      break
  f.close()
  if filename != newfilename:
    print "   Renaming %s to %s/%s" %(filename, os.path.dirname(filename), newfilename)
    os.rename(filename, "%s/%s" %(os.path.dirname(filename), newfilename))
    #os.unlink(filename)
  else:
    print "   Couldn't find name in %s" %filename

def getHeaderData(data):
  headers = []
  while len(data) > 0:
    type = ord(data[0])
    size = struct.unpack(">H", data[1:3])[0]
    o = data[3:size]
    data = data[size:]
    td = parseHeader(type, o)
    headers.append(td)
    if td["type"] == 0:
      #print "Header Size: %s" % td["headerlength"]
      data = data[:td["headerlength"]-size]
  return headers

def parseHeader(type, data):
  if type == 0:
    filetypecode, headerlength, datalength = struct.unpack(">BIQ", data)
    return {"type":type, "filetypecode":filetypecode, "headerlength":headerlength, "datalength":datalength}
  elif type == 1:
    bitsperpixel, columns, lines, compression = struct.unpack(">BHHB", data)
    return {"type":type, "bitsperpixel":bitsperpixel, "columns":columns, "lines":lines, "compression":compression}

  elif type == 2:
    projname, cfac, lfac, coff, loff = struct.unpack(">32sIIII", data)
    return {"type":type, "projname":projname, "cfac":cfac, "lfac":lfac, "coff":coff, "loff":loff}

  elif type == 3:
    return {"type":type, "data":data}

  elif type == 4:
    return {"type":type, "filename":data}

  elif type == 5:
    days, ms = struct.unpack(">HI", data[1:])
    return {"type":type, "days":days, "ms":ms}

  elif type == 6:
    return {"type":type, "data":data}

  elif type == 7:
    return {"type":type, "data":data}

  elif type == 128:
    imageid, sequence, startcol, startline, maxseg, maxcol, maxrow = struct.unpack(">7H", data)
    return {"type":type, "imageid":imageid, "sequence":sequence, "startcol":startcol, "startline":startline, "maxseg":maxseg, "maxcol":maxcol, "maxrow":maxrow}

  elif type == 129:
    signature, productId, productSubId, parameter, compression = struct.unpack(">4sHHHB", data)
    return {"type":type, "signature":signature, "productId":productId, "productSubId":productSubId, "parameter":parameter, "compression":compression}

  elif type == 130:
    return {"type":type, "data":data}

  elif type == 131:
    flags, pixel, line = struct.unpack(">HBB", data)
    return {"type":type, "flags":flags, "pixel":pixel, "line":line}

  elif type == 132:
    return {"type":type, "data": data}
  else:
    return {"type":type}

def readHeader(f):
  global t
  type = ord(f.read(1))
  size = f.read(2)
  size = struct.unpack(">H", size)[0]
  data = f.read(size-3)

  if type == 0:
    filetypecode, headerlength, datalength = struct.unpack(">BIQ", data)
    return type, filetypecode, headerlength, datalength
  elif type == 1:
    bitsperpixel, columns, lines, compression = struct.unpack(">BHHB", data)
    return type, bitsperpixel, columns, lines, compression

  elif type == 2:
    projname, cfac, lfac, coff, loff = struct.unpack(">32sIIII", data)
    return type, projname, cfac, lfac, coff, loff

  elif type == 3:
    return type, data

  elif type == 4:
    return type, data

  elif type == 5:
    days, ms = struct.unpack(">HI", data[1:])
    return type, days, ms

  elif type == 6:
    return type, data

  elif type == 7:
    return type, data

  elif type == 128:
    imageid, sequence, startcol, startline, maxseg, maxcol, maxrow = struct.unpack(">7H", data)
    return type, imageid, sequence, startcol, startline, maxseg, maxcol, maxrow

  elif type == 129:
    signature, productId, productSubId, parameter, compression = struct.unpack(">4sHHHB", data)
    return type, signature, productId, productSubId, parameter, compression

  elif type == 130:
    return type, data

  elif type == 131:
    flags, pixel, line = struct.unpack(">HBB", data)
    return type, flags, pixel, line

  elif type == 132:
    return type, data

  else:
    return type

def printHeaders(headers, showStructuredHeader=False, showImageDataRecord=False):
  for head in headers:
    type = head["type"]
    if type == 0:
      print "Header type: %s File Type Code: %s Header Length %s Data Field Length: %s" %(type, head["filetypecode"], head["headerlength"], head["datalength"])
    elif type == 1:
      print "Image Structure Header: "
      print "   Bits Per Pixel: %s" %head["bitsperpixel"]
      print "   Columns: %s" %head["columns"]
      print "   Lines: %s" %head["lines"]
      print "   Compression: %s" %head["compression"]

    elif type == 2:
      print "Image Navigation Record"
      print "   Projection Name: %s" %head["projname"]
      print "   Column Scaling Factor: %s" %head["cfac"]
      print "   Line Scaling Factor: %s" %head["lfac"]
      print "   Column Offset: %s" %head["coff"]
      print "   Line Offset: %s" %head["loff"]

    elif type == 3:
      print "Image Data Function Record"
      if showImageDataRecord:
        print "   Data: %s" %head["data"]
      else:
        print "   Data: {HIDDEN}"

    elif type == 4:
      print "Annotation Record"
      print "   Filename: %s" %head["filename"]

    elif type == 5:
      print "Timestamp Record"
      print "   Delta from 1 January 1958"
      print "     Days: %s" %head["days"]
      print "     Miliseconds: %s" %head["ms"]

    elif type == 6:
      print "Ancillary Text"
      print "   Data: "
      t = head["data"].split(";")
      for i in t:
        print "     %s" %i

    elif type == 7:
      print "Key Header"
      print "   Data: %s" %head["data"]

    elif type == 128:
      print "Segment Identification Header"
      print "   Image Id: %s" %head["imageid"]
      print "   Sequence: %s" %head["sequence"]
      print "   Start Column: %s" %head["startcol"]
      print "   Start Line: %s" %head["startline"]
      print "   Number of Segments: %s" %head["maxseg"]
      print "   Width: %s" %head["maxcol"]
      print "   Height: %s" %head["maxrow"]

    elif type == 129:
      print "NOAA Specific Header"
      print "   Signature: %s" %head["signature"]
      print "   Product ID: %s" %head["productId"]
      print "   Product SubId: %s" %head["productSubId"]
      print "   Parameter: %s" %head["parameter"]
      print "   Compression: %s" %head["compression"]

    elif type == 130:
      print "Header Structured Record"
      if showImageDataRecord:
        t = head["data"].split("UI")
        print "   Data: "
        for i in t:
          print "     %s" %i
      else:
        print "   Data: {HIDDEN}"

    elif type == 131:
      print "Rice Compression Record"
      print "   Flags: %s" %head["flags"]
      print "   Pixel: %s" %head["pixel"]
      print "   Line: %s" %head["line"]

    elif type == 132: # Got in DCS Data
      print "DCS Data: "
      print "   Data: %s" %head["data"]

    else:
      print "Type not mapped: %s" % type
    print ""