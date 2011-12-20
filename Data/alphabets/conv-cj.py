#!/usr/bin/python

import sys
import re
import string

def die(m):
  print "**ERROR**",m
  sys.exit(1)

SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;

def surrog(low,high):
  r='&#x'+string.upper(hex((low << 10) + high + SURROGATE_OFFSET)[2:])+';'
  #print "<!--Note: encoding",xml_hex(unichr(low)),xml_hex(unichr(high)),"as",r,'-->'
  return r

def xml_hex(s):
  if len(s)==1:
    return '&#x'+string.upper(hex(ord(s))[2:])+';' #hex() returns "0x..."
  if len(s)==2:
    if ord(s[0])>=0xD800 and ord(s[0])<=0xDBFF and ord(s[1])>=0xDC00 and ord(s[1])<=0xDFFF:
      return surrog(ord(s[0]),ord(s[1]));
    raise IndexError(s.encode("utf-8")+" is two bytes but not a surrogate")
  die(s.encode("utf-8")+" of length "+len(s)+" not 1 or 2");

#// computations
#UTF16 lead = LEAD_OFFSET + (codepoint >> 10);
#UTF16 trail = 0xDC00 + (codepoint & 0x3FF);

#UTF32 codepoint = (lead << 10) + trail + SURROGATE_OFFSET;

#rows on qwerty keyboard, with xml colour spec. We sort groups+radicals by this
# (at each level), and group each row together in a group of that colour
rows = [ ('!@#$%^&*()-_=+',"60"), ('qwertyuiop[{]}',"61"), ('asdfghjkl;:\'"\\|',"62"), (r'`~zxcvbnm,<.>/?',"63") ];
#map from qwerty key, to cangjie radical
keys = {}

#tree structure. strokes = all the ciangxie symbols; keyname = all the keys (a-z0-9 etc.)
class Group:
  def __init__(self,strokes="",keyname=""):
    self.syms= set()
    self.strokes=strokes
    self.keyname=keyname
    self.children = {}
  
  def getChild(self, ch, key):
    if (ch not in self.children):
      self.children[ch] = Group(self.strokes+ch,self.keyname+key);
    return self.children[ch];

  def recursiveDelete(self,syms):
    self.syms = self.syms - syms;
    for ch in self.children.itervalues():
      ch.recursiveDelete(syms)
    self.children = dict((k,v) for k,v in self.children.iteritems() if (len(v.syms)>0 or len(v.children)>0))
  
  #list should contain single chars - just to put them in order
  def write(self, indent):
    #first, all direct children
    for sym in sorted(self.syms):      
      s=xml_hex(sym)
      print indent + '<s d="'+s+'" t="'+s+'"/>';
    #then subgroups, in row order:
    for row,col in rows:
      mine = [keys[k] for k in row if keys[k] in self.children.keys()];
      if len(mine)==0: continue
      print indent + '<group b="'+col+'">';
      for sym in mine:
        n=xml_hex(sym) 
        print indent + ' <group label="'+n+'">'
        self.children[sym].write(indent+"  ")
        print indent + " </group>";
      print indent + "</group>";

  def addSym(self,sym):
    self.syms.add(sym)

  def __str__(self):
    return "group "+self.strokes.encode("utf-8")+" ("+self.keyname.encode("utf-8")+")"

keypat = re.compile("([^ ]+) +([^ \r\n]+)",re.UNICODE);

with open(sys.argv[1]) as file:
  for line in file:
    if (re.match("%keyname",line) is not None):
      break
  #line should be the keyname begin.
  for line in file:
    if (re.match("%keyname",line) is not None): break
    m = keypat.match(unicode(line,"utf-8"));
    if (m is None):
      die("Error, could not parse keyname"+line)
    else:
      keys[m.group(1)] = m.group(2);
  #line should be the keyname end

  for line in file:
    if (re.match("%chardef +begin",line) is not None):
      break
    die("Expected chardef begin, got "+line)
  punctuation=set() #symbols in range we think means punctuation, that exist
  root = Group()
  pattern=re.compile("([^ \s\t]+)[\s\t]+([^ \s\t]{0,2})$",re.UNICODE);
  for line in file:
    if (re.match("%chardef +end",line) is not None): break
    m=pattern.match(unicode(line,"utf-8"))
    if m is None:
      die("matching "+line)
    sym =(m.group(2) if m.group(2) else " ")
    try:
      xml_hex(sym)
    except IndexError as e:
      print '<!--WARNING: Skipping',e,'-->'
      continue
    o=ord(sym[0])  
    if (o>0x2000 and o<0x2036) or (o>0x3000 and o<0x301F) or (o>0xFE3C and o<0xFFFF):
      punctuation.add(sym)
      continue
    key = m.group(1)
    r=root
    for ch in key:
      r = r.getChild(keys[ch],ch)
    r.addSym(sym)
#don't create a space character (or put in punctuation), Dasher will add itself
root.recursiveDelete(punctuation | set(" "))
root.write("")
print '<group name="Punctuation" b="112">'
for sym in sorted(punctuation):
  s = xml_hex(sym)
  print '  <s t="'+s+'" d="'+s+'" />'
print "</group>"
