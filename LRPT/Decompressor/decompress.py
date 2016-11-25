#!/usr/bin/env python

'''
  Python version of LRPT Frame Decompressor
'''

import struct

class PassNode:
  left = None
  right = None

  def printTree(self, spaces=0):
    print " " * (spaces + 1) + "Pass Node"
    if self.left != None:
      print " " * (spaces + 1) +"0: "
      self.left.printTree(spaces + 1)
    #else:
    #  print " " * (spaces + 1) + "None"
    if self.right != None:
      print " " * (spaces + 1) +"1: "
      self.right.printTree(spaces + 1)

class DCHuffNode:
  codeword = ""
  codelength = 0
  category = 0
  cVal = 0

  def __init__(self, codeword, codelength, category):
    #print "Creating node with %s %s %s" %(codeword, codelength, category)
    self.codeword = codeword
    self.codelength = codelength
    self.category = category

  def printTree(self, spaces=0):
    print " " * (spaces + 1) + ("Category %s" %self.category)

class ACHuffNode:
  run = 0
  size = 0
  codeword = ""

  def __init__(self, codeword, run, size):
    self.codeword = codeword
    self.run = run
    self.size = size

  def printTree(self, spaces=0):
    print " " * (spaces + 1) + ("Run/Size (%s/%s)" %(self.run, self.size))

  def isEoB(self):
    return self.run == 0 and self.size == 0

  def isZRL(self):
    self.run == 16 and self.size == 0

# Build DC Tree
# Left = 0, Right = 1
dc_codewords = [
  {"category": 0, "codeword": "00"},
  {"category": 1, "codeword": "010"},
  {"category": 2, "codeword": "011"},
  {"category": 3, "codeword": "100"},
  {"category": 4, "codeword": "101"},
  {"category": 5, "codeword": "110"},
  {"category": 6, "codeword": "1110"},
  {"category": 7, "codeword": "11110"},
  {"category": 8, "codeword": "111110"},
  {"category": 9, "codeword": "1111110"},
  {"category":10, "codeword": "11111110"},
  {"category":11, "codeword": "111111110"}
]

dcroot = PassNode()

for n in dc_codewords:
  tmproot = dcroot
  i = 0
  bits = len(n["codeword"])
  for b in n["codeword"]:
    if i == bits-1:
      if b == "0":
        if tmproot.left != None:
          print "ERROR!"
        tmproot.left = DCHuffNode(n["codeword"], bits, n["category"])
      else:
        if tmproot.right != None:
          print "ERROR!"
        tmproot.right = DCHuffNode(n["codeword"], bits, n["category"])
    else:
      if b == "0":
        if tmproot.left == None:
          tmproot.left = PassNode()
        tmproot = tmproot.left
      else:
        if tmproot.right == None:
          tmproot.right = PassNode()
        tmproot = tmproot.right
    i += 1

# Build AC Tree
ac_codewords = [
  {"run":0, "size":   0, "codeword": "1010"},
  {"run":0, "size":   1, "codeword": "00"},
  {"run":0, "size":   2, "codeword": "01"},
  {"run":0, "size":   3, "codeword": "100"},
  {"run":0, "size":   4, "codeword": "1011"},
  {"run":0, "size":   5, "codeword": "11010"},
  {"run":0, "size":   6, "codeword": "1111000"},
  {"run":0, "size":   7, "codeword": "11111000"},
  {"run":0, "size":   8, "codeword": "1111110110"},
  {"run":0, "size":   9, "codeword": "1111111110000010"},
  {"run":0, "size": 0xA, "codeword": "1111111110000011"},
  {"run":1, "size":   1, "codeword": "1100"},
  {"run":1, "size":   2, "codeword": "11011"},
  {"run":1, "size":   3, "codeword": "1111001"},
  {"run":1, "size":   4, "codeword": "111110110"},
  {"run":1, "size":   5, "codeword": "11111110110"},
  {"run":1, "size":   6, "codeword": "1111111110000100"},
  {"run":1, "size":   7, "codeword": "1111111110000101"},
  {"run":1, "size":   8, "codeword": "1111111110000110"},
  {"run":1, "size":   9, "codeword": "1111111110000111"},
  {"run":1, "size": 0xA, "codeword": "1111111110001000"},
  {"run":2, "size":   1, "codeword": "11100"},
  {"run":2, "size":   2, "codeword": "11111001"},
  {"run":2, "size":   3, "codeword": "1111110111"},
  {"run":2, "size":   4, "codeword": "111111110100"},
  {"run":2, "size":   5, "codeword": "1111111110001001"},
  {"run":2, "size":   6, "codeword": "1111111110001010"},
  {"run":2, "size":   7, "codeword": "1111111110001011"},
  {"run":2, "size":   8, "codeword": "1111111110001100"},
  {"run":2, "size":   9, "codeword": "1111111110001101"},
  {"run":2, "size": 0xA, "codeword": "1111111110001110"},
  {"run":3, "size":   1, "codeword": "111010"},
  {"run":3, "size":   2, "codeword": "111110111"},
  {"run":3, "size":   3, "codeword": "111111110101"},
  {"run":3, "size":   4, "codeword": "1111111110001111"},
  {"run":3, "size":   5, "codeword": "1111111110010000"},
  {"run":3, "size":   6, "codeword": "1111111110010001"},
  {"run":3, "size":   7, "codeword": "1111111110010010"},
  {"run":3, "size":   8, "codeword": "1111111110010011"},
  {"run":3, "size":   9, "codeword": "1111111110010100"},
  {"run":3, "size": 0xA, "codeword": "1111111110010101"}
]

acroot = PassNode()

for n in ac_codewords:
  tmproot = acroot
  i = 0
  bits = len(n["codeword"])
  for b in n["codeword"]:
    if i == bits-1:
      if b == "0":
        if tmproot.left != None:
          print "ERROR!"
        tmproot.left = ACHuffNode(n["codeword"], n["run"], n["size"])
      else:
        if tmproot.right != None:
          print "ERROR!"
        tmproot.right = ACHuffNode(n["codeword"], n["run"], n["size"])
    else:
      if b == "0":
        if tmproot.left == None:
          tmproot.left = PassNode()
        tmproot = tmproot.left
      else:
        if tmproot.right == None:
          tmproot.right = PassNode()
        tmproot = tmproot.right
    i += 1


def binary(num, length=8):
    return format(num, '#0{}b'.format(length + 2))

def findDCCategory(bdata, len):
  global dcroot
  bdata = binary(bdata, len)[2:]
  tmproot = dcroot
  for i in bdata:
    if i == "0":
      tmproot = tmproot.left
    else:
      tmproot = tmproot.right
    if tmproot == None:
      break
  return tmproot if not isinstance(tmproot, PassNode) else None

def findACCategory(bdata, len):
  global acroot
  bdata = binary(bdata, len)[2:]
  tmproot = acroot
  for i in bdata:
    if i == "0":
      tmproot = tmproot.left
    else:
      tmproot = tmproot.right
    if tmproot == None:
      break
  return tmproot if not isinstance(tmproot, PassNode) else None

def mapRange(category, value):
  maxVal = (2 ** category) - 1
  sig = (value >> (category - 1))
  if sig: # Positive number
    return value
  else:                       # Negative number
    return value - maxVal

# Now the processing

f = open("Debug/66_0_7329.lrpt", "rb")
header = f.read(14)
data = f.read()
f.close()

day, msFromDay, iss = struct.unpack(">HIH", header[:8])
header = header[8:]

nmcu, qt, acdc, qfm, qFactor = struct.unpack(">BBBHB", header)

dc = (acdc & 0xF0) >> 4
ac = acdc & 0xF0

print "Day: %s" %day
print "Milisseconds from Day: %s" %msFromDay
print "ISS(?): %s" %iss
print "NMCU: %s" %nmcu
print "QT: %s" %qt
print "DC: %s" %dc
print "AC: %s" %ac
print "QFM: %s" %qfm
print "Q: %s" %qFactor

# Decode DC

d = struct.unpack(">H", data[:2])[0]

bytepos = 2
pos = 2
dccat = -1
curbits = 16
while True:
  kd = d >> (16 - pos)
  r = findDCCategory(kd, pos)
  if r != None and not isinstance(r, PassNode):
    dccat = r.category
    break
  pos += 1
  if pos == 16:
    print "Not found"
    break

if dccat == -1:
  print "Corrupted?"
  exit(1)

if 16 - pos - dccat < 0:
  print "Not enough data. Fetching one more byte"
  d = d << 8
  d |= ord(data[bytepos])
  bytepos += 1

# Remove the pos bits of the encoding
curbits -= pos
mask = (2**curbits)-1
d = d & ~(mask << (curbits)) # Remove the pos bits of the encoding
v = d >> (curbits - dccat)
print binary(d, curbits)

currDC = mapRange(dccat, v) - dc

# Remove the dccat bits from the encoding
print curbits
mask = (2**curbits)-1
curbits -= dccat
d = d & ~(mask << (curbits)) # Remove the pos bits of the encoding

print "Block DC: %s"%currDC

acs = []

# OK until here
u = 0
# Decode ACs
while True:
  u+=1
  while curbits < 16: # Always Keep more than 16 bits in buffer
    d = d << 8
    d |= ord(data[bytepos])
    bytepos += 1
    curbits += 8

  ac = None
  pos = 2
  while True:
    kd = d >> (curbits - pos)
    r = findACCategory(kd, pos)
    if r != None and not isinstance(r, PassNode):
      ac = r
      break
    pos += 1
    if pos == curbits:
      print "Not found"
      break

  if ac == None:
    print "Breaking! Not found"
    print binary(d)
    break

  if ac.run == 0 and ac.size == 0:
    print "End of Block"
    if len(acs) < 63:
      for i in range(len(acs), 63):
        acs.append(0)
    break

  if ac.run > 0:
    print "Filling run %s in matrix" %ac.run
    for i in range(ac.run):
      acs.append(0)

  print "Remaining bits: %s" %binary(d, curbits)
  print "Stripping code head of %s bits" %pos
  curbits -= pos
  mask = (2**curbits)-1
  d = d & ~(mask << (curbits)) # Remove the pos bits of the encoding

  print "Remaining bits: %s" %binary(d, curbits)

  print "Reading %s bits" % ac.size
  v = d >> (curbits - ac.size)
  # Remove the data we just read
  curbits -= ac.size
  mask = (2**curbits)-1
  d = d & ~(mask << (curbits)) # Remove the pos bits of the encoding

  print "Size: %s Val: %d" %(ac.size, v)
  val = mapRange(ac.size, v)
  acs.append(val)
  print binary(d, curbits)
  print val

print acs, len(acs), u