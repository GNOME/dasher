#!/usr/bin/python

import sys
import re
import string

def regor(xs):
  return "["+("".join([("\\." if x is "." else x) for x in xs]))+"]"

#tree structure: top level chooses between initials (perhaps blank), next level groups, and so on
class Group:
  def __init__(self):
    self.syms = [];
    self.children = {};
  
  def getChild(self, ch):
    if (ch not in self.children):
      self.children[ch] = Group();
    return self.children[ch];
  
  #lists should contain (a list of ordered initials), then (a list of groups), then finals, then tones,
  # but only starting with whichever will be required from this level of the tree down.
  def write(self, indent, name, lists):
    if len(lists)==0:
      for sym in self.syms:
        s='&#x'+hex(ord(sym))[2:]+';'
        print indent + '<s d="'+s+'" t="'+s+'"/>';
    else:
      for ch in lists[0]:
        if ch not in self.children:
          continue;
        if ch=="":
          n=""
          desc=indent + '<group label="" visible="no"'
        else:
          n='&#x'+string.upper(hex(ord(ch))[2:])+';' #hex() returns "0x..."
          desc=indent + '<group label="'+n+'"'
        if self.children[ch].syms:
          desc+=' name="'+name+n+'"'
        print desc+'>'
        self.children[ch].write(indent+" ",name+n,lists[1:]);
        del self.children[ch]
        print indent + "</group>";
    for ch in self.children:
      print "WARNING Did not output",(["tone","final","group","initial"][len(lists)-1]),ch.encode("utf-8")

  def addSym(self,sym):
    self.syms+=sym

keypat = re.compile("([^ ]+) +([^ \r\n]+)",re.UNICODE);
keys = {}

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

  groups = []
  initials = []
  finals = []
  tones = []

  for key,value in keys.items():
    v=ord(value)
    if v==0x3127 or v==0x3128 or v==0x3129:
      groups+=key
    elif v>=0x3105 and v<0x311A:
      initials+=key
    elif v>=0x311A and v<=0x3127:
      finals+=key
    elif v==0x02CA or v==0x02C7 or v==0x02CB or v==0x02D9: #0x30FB is a big version of 02d9
      tones+=key
    else:
      die("**ERROR** unknown bopomofo "+key+" = "+value)
  pat = "("+regor(initials)+"?)("+regor(groups)+"?)("+regor(finals)+"?)("+regor(tones)+"?)[\s\t]*([^\s\t]+)\n"
  #print pat
  pattern=re.compile(pat,re.UNICODE);

  for line in file:
    if (re.match("%chardef +begin",line) is not None):
      break
    die("Expected chardef begin, got "+line)
  
  root = Group()
  keys[""]=""
  for line in file:
    if (re.match("%chardef +end",line) is not None): break
    m=pattern.match(unicode(line,"utf-8"))
    if m is not None:
      root.getChild(keys[m.group(1)]).getChild(keys[m.group(2)]).getChild(keys[m.group(3)]).getChild(keys[m.group(4)]).addSym(m.group(5))
    else:
      die("**ERROR** matching "+line)

def ch(lst):
  lst=[keys[x] for x in lst];
  lst.sort()
  lst.insert(0,"")
  return lst

root.write("","",[ch(x) for x in [initials,groups,finals,tones]])
