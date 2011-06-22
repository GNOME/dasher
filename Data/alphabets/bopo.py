#!/usr/bin/python

import re
import sys

if len(sys.argv)!=3:
  print "Usage: bopo.py <path to bpfm-pinyin.txt> <path to alphabet.spyDict.xml>"
  print "Prints out group structure for alphabet.bopoDict.xml, excluding punctuation etc."
  print "(Also WARNING messages, so look for these first - the ones saying it's skipping punctuation, etc., are fine)"
  sys.exit(1);

#a bopomofo sound, is an initial, a group, a final, and a tone, in that order. Any may be null,
# in which case we record it explicitly with the empty string. Build lists of initials+groups+finals+tones, in order...

initials = [""];
for init in range(0x3105, 0x311A):
  initials.append(unichr(init));

groups = ["", u"\u3127",u"\u3128",u"\u3129"]

finals=[""]
for fin in range(0x311A, 0x3127):
  finals.append(unichr(fin))

#the accent / tone-mark character for the tones as numbered in pinyin
# (in bopomofo, tone 1 is indicated by the absence of a mark, and tone 5 is a dot,
#  whereas in pinyin, tone 1 is a horizontal bar and tone 5 is unwritten)
marks = ['ONE-INDEXED', '', u'\u02CA', u'\u02C7', u'\u02CB', u'\u30FB'];

#tree structure: top level chooses between initials (perhaps blank), next level groups, and so on
class Group:
  def __init__(self):
    self.sym = "";
    self.children = {};
  
  def getChild(self, ch):
    if (ch not in self.children):
      self.children[ch] = Group();
    return self.children[ch];
  
  #lists should contain (a list of ordered initials), then (a list of groups), then finals, then tones,
  # but only starting with whichever will be required from this level of the tree down.
  def write(self, indent, lists):
    if (self.sym is not ""):
      print indent, self.sym,
    if len(lists)==0:
      return
    for ch in lists[0]:
      if ch not in self.children:
        continue;
      if ch=="":
        print indent + '<group label="" visible="no">'
      else:
        print indent + '<group label="&#x'+hex(ord(ch))[2:]+';">' #hex() returns "0x..."
      self.children[ch].write(indent+" ",lists[1:]);
      del self.children[ch]
      print indent + "</group>";
    for ch in self.children:
      print "WARNING Did not output",(["tone","final","group","initial"][len(lists)-1]),ch
  
  def setSym(self, sym):
    if (self.sym != ""):
      print "WARNING Overwriting",self.sym,"with",sym
    self.sym=sym;

#read in the mapping from bopomofo to pinyin - see bpmf-pinyin.txt
pyToBPFM = {};
pattern=re.compile("(["+("".join(initials))+"]?)(["+("".join(groups))+"]?)(["+("".join(finals))+u"]?)[\s\t]*([a-z\u00FC]*)\n",re.UNICODE);
with open(sys.argv[1],'r') as file:
  for line in file:
    if (line[0]=='#'): continue #simple format for comments
    m=pattern.match(unicode(line,"utf-8"));
    if m is None:
      print "WARNING Could not parse",line,"starting with",ord(line[0])
      continue
    pyToBPFM[m.group(4)] = (m.group(1),m.group(2),m.group(3));

#read in the spyDict file. For each leaf group, i.e. with tone,
# we extract the equivalent bopomofo and remember that + the tone, for the symbol
# within it. We then store the symbol into the Group tree according to the parsed bopomofo.
root=Group()
pattern = re.compile('<group name="([a-z]+)([1-5]?)"');
with open(sys.argv[2],'r') as file:
  for line in file:
    if re.match("<group",line) is not None:
      m=pattern.match(line);
      if m is None:
        print "WARNING Did not parse group",line,
        ready=False
        continue
      if (m.group(2)=="") or (m.group(1)=="hng") or (m.group(1)=="m"):
        ready=False
        continue #special case
      (i,g,f) = pyToBPFM[m.group(1)];
      t = marks[int(m.group(2))];
      ready=True
    elif re.match("<s ",line) is not None:
      #if (re.search("554A",line) is not None) or (re.search("82AD",line) is not None):
      #  print "Sym",line,"with initial",i.encode("utf-8"),"group",g.encode("utf-8"),"final",f.encode("utf-8"),"tone",t,ready
      if ready:
        root.getChild(i).getChild(g).getChild(f).getChild(t).setSym(line);
      #else, it's a number, or similar

#output the tree of groups...
root.write("",[initials,groups,finals,marks])
#NOTE the hack for pinyin m2, you'll have to do this by hand afterwards
# (and also put the tree into alphabet.bopoDict.xml in the right place, after the intro
#  and before punctuation etc.)
