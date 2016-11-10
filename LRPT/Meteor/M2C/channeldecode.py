#!/usr/bin/env python
'''
    LRPT Channel Decoder / Demuxer
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

import sys, struct, os
from subprocess import call

FRAMESIZE = 892
M_PDUSIZE = FRAMESIZE - 6

tsize = 0
startnum = -1
endnum = -1

SEQUENCE_FLAG_MAP = {
  0: "Continued Segment",
  1: "First Segment",
  2: "Last Segment",
  3: "Single Data"
}

def ParseMSDU(data):
  o = struct.unpack(">H", data[:2])[0]
  version = (o & 0xE000) >> 13
  type = (o & 0x1000) >> 12
  shf = (o & 0x800) >> 11
  apid = (o & 0x7FF)

  o = struct.unpack(">H", data[2:4])[0]
  sequenceflag = (o & 0xC000) >> 14
  packetnumber = (o & 0x3FFF)
  packetlength = struct.unpack(">H", data[4:6])[0] -1
  data = data[6:]
  return version, type, shf, apid, sequenceflag, packetnumber, packetlength, data

def SavePacket(channelid, packet):
  global totalSavedPackets
  global tsize
  global startnum
  global endnum

  #packet["sequenceflag_int"] = 3 # TEST
  try:
    os.mkdir("channels/%s" %channelid)
  except:
    pass

  if packet["apid"] == 2047:
    print "  Fill Packet. Skipping"
    return

  datasize = len(packet["data"])

  if not datasize - 2 == packet["size"]: # CRC is the latest 2 bytes of the payload
    print "   WARNING: Packet Size does not match! Expected %s Found: %s" %(packet["size"], len(packet["data"]))
    if datasize - 2 > packet["size"]:
      datasize = packet["size"] + 2
      print "   WARNING: Trimming data to %s" % datasize

  data = packet["data"][:datasize-2]

  if packet["sequenceflag_int"] == 1:
    print "Starting packet %s_%s_%s.lrpt"  % (packet["apid"], packet["version"], packet["packetnumber"])
    startnum = packet["packetnumber"]
  elif packet["sequenceflag_int"] == 2:
    print "Ending packet %s_%s_%s.lrpt"  % (packet["apid"], packet["version"], packet["packetnumber"])
    endnum = packet["packetnumber"]
    if startnum == -1:
      print "Orphan Packet. Dropping"
      return
  elif packet["sequenceflag_int"] != 3 and startnum == -1:
    print "Orphan Packet. Dropping."
    return

  if packet["framesdropped"]:
    print "   WARNING: Some frames has been droped for this packet."


  filename = "channels/%s/%s_%s_%s.lrpt" % (channelid, packet["apid"], packet["version"], packet["packetnumber"])
  #print "- Saving packet to %s" %filename

  firstorsinglepacket = packet["sequenceflag_int"] == 1 or packet["sequenceflag_int"] == 3
  f = open(filename, "wb")

  f.write(data)
  f.close()

  if firstorsinglepacket:
    tsize = packet["size"]
  else:
    tsize += packet["size"]

  totalSavedPackets += 1

def CreatePacket(data):
  while True:
    if len(data) < 6:
      return -1, data
    version, type, shf, apid, sequenceflag, packetnumber, packetlength, data = ParseMSDU(data)
    pdata = data[:packetlength+2]
    if apid != 2047:
      pendingpackets[apid] = {
        "data": pdata,
        "version": version,
        "type": type,
        "apid": apid,
        "sequenceflag": SEQUENCE_FLAG_MAP[sequenceflag],
        "sequenceflag_int": sequenceflag,
        "packetnumber": packetnumber,
        "framesdropped": False,
        "size": packetlength
      }
      #print "- Creating packet %s Size: %s - %s" % (apid, packetlength, SEQUENCE_FLAG_MAP[sequenceflag])
    else:
      apid = -1

    if not packetlength+2 == len(data) and packetlength+2 < len(data): # Multiple packets in buffer
      SavePacket(sys.argv[1], pendingpackets[apid])
      del pendingpackets[apid]
      data = data[packetlength+2:]
      apid = -1
      #print "   Multiple packets in same buffer. Repeating."
    else:
      break
  return apid, ""


if len(sys.argv) < 2:
  print "Usage: ./channeldecode.py CHANNELID"
  print "This will open channels/channel_CHANNELID.bin"
  exit()

filename = "channels/channel_%s.bin" % sys.argv[1]

f = open(filename, "r")
fsize = os.path.getsize(filename)
readbytes = 0

pendingpackets = {}

lastFrameNumber = -1
totalFrameDrops = 0
totalCRCErrors = 0
totalSavedPackets = 0
lastAPID = -1
buff = ""

while readbytes < fsize:
  if fsize - readbytes < FRAMESIZE:
    print "   Some bytes at end of file was not enough for filling a frame. Remaining Bytes: %s - Frame Size: %s" % (fsize-readsize, FRAMESIZE)
    break

  # Read Data
  data = f.read(FRAMESIZE)
  versionNumber = (ord(data[0]) & 0xC0) >> 6
  scid = (ord(data[0]) & 0x3F) << 2 | (ord(data[1]) & 0xC0) >> 6
  vcid = (ord(data[1]) & 0x3F)

  counter = struct.unpack(">I", data[2:6])[0]
  counter &= 0xFFFFFF00
  counter >>= 8

  # Check for dropped Frames
  if not lastFrameNumber == -1 and not lastFrameNumber+1 == counter:
    print "   Frames dropped: %s" % (counter-lastFrameNumber-1);
    totalFrameDrops += counter-lastFrameNumber-1;
    if not lastAPID == -1: # Fill
      pendingpackets[lastAPID]["framesdropped"] = True


  #print "SC: %s ID: %s Frame Number: %s" % (scid, vcid, counter)

  # Demux M_PDU
  data = data[6:] # Strip channel header

  in_sdu = struct.unpack("BB", data[:2])
  if in_sdu[0] != 0:
    print "Encrypted packet"

  data = data[2:]
  fhp = struct.unpack(">H", data[:2])[0] & 0x7FF
  data = data[2:] # Strip M_PDU Header
  #print "   First Packet Header: %s" %fhp
  #data is now TP_PDU
  if not fhp == 2047: # Frame Contains a new Packet
    # Data was incomplete on last FHP and another packet starts here.
    if lastAPID == -1 and len(buff) > 0:
      #print "   Data was incomplete from last FHP. Parsing packet now"
      if fhp > 0:
        buff += data[:fhp]
      lastAPID, data = CreatePacket(buff)
      if lastAPID == -1:
        buff = data
      else:
        buff = ""

    if not lastAPID == -1: # We are finishing another packet
      #print "Finishing last packet with APID %s" %lastAPID
      if fhp > 0:
        pendingpackets[lastAPID]["data"] += data[:fhp]
      SavePacket(sys.argv[1], pendingpackets[lastAPID])
      del pendingpackets[lastAPID]
      lastAPID = -1

    # Try to create a new packet
    buff += data[fhp:]
    lastAPID, data = CreatePacket(buff)
    if lastAPID == -1:
      buff = data
    else:
      buff = ""
  else:
      if len(buff) > 0 and lastAPID == -1:
        #print "   Data was incomplete from last FHP. Parsing packet now"
        buff += data
        lastAPID, data = CreatePacket(buff)
        if lastAPID == -1:
          buff = data
        else:
          buff = ""
      elif len(buff) > 0:
        print "   PROBLEM!"
      elif lastAPID == -1:
        buff += data
        lastAPID, data = CreatePacket(buff)
        if lastAPID == -1:
          buff = data
        else:
          buff = ""
      else:
        #print "   Appending %s bytes to %s" % (lastAPID, len(data))
        pendingpackets[lastAPID]["data"] += data


  lastFrameNumber = counter
  readbytes += FRAMESIZE

# One packet can be still in pending packets
for i in pendingpackets.keys():
  SavePacket(sys.argv[1], pendingpackets[lastAPID])

print "\n\nReport:"
print "\tTotal Frames Dropped: %s" %totalFrameDrops
print "\tTotal Saved Packets: %s" %totalSavedPackets

f.close()