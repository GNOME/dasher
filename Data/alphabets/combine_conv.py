#!/usr/bin/python

import sys
import re

if (len(sys.argv)!=3):
  print "Usage: combine_conv.py /path/to/alphabet.chineseRuby.xml /path/to/alphabet.spyDict.xml"
  print "Reads conversions from former and inlines into latter, output to stdout"
  sys.exit(1)

groups={}
curgroup=None
labpat = re.compile(r'label="([^"]+)"')
with open(sys.argv[1]) as file:
  for line in file:
    if (re.match("<group",line) is not None):
      m = labpat.search(line)
      if (m is not None): curgroup = m.group(1)
    elif (re.match("</group",line) is not None):
      #print "Parsed",curgroup,groups[curgroup]
      curgroup=None
    elif curgroup is not None:
      groups[curgroup]=(groups[curgroup]+line) if (curgroup in groups) else line
    else:
      
      print"<!--Skipping",line.rstrip("\n"),"-->"
#sys.exit(0)
labpat=re.compile(r'd="([^"]+)"')
namepat = re.compile(r'name="[^"]+"')
bPunc = False
lastGroup=""
with open(sys.argv[2]) as file:
  for line in file:
    if (re.match("\s*<group",line) is not None):
      print lastGroup,
      lastGroup=line
      if (re.search('name="Punctuation"',line) is not None):
        bPunc=True
      continue; #avoid printing (until next iter)
    elif (re.match("\s*<s ",line) is not None):
      m=labpat.search(line)
      if m is None:
        print "*ERROR* Could not find label in",line
      elif m.group(1) in groups:
        #substitute in the body of the alphabet.chineseRuby group corresponding to this symbol:
        line = groups[m.group(1)]
        if bPunc:
          #in alphabet.spyDict.xml the punctuation symbols are all together in the same group.
          #Each identifies a single punctuation symbol in the CH alphabet, but we need
          # to put that in its own group, so the PY language model assigns a PY probability
          # to each punctuation symbol separately, rather than all punctuation together.
          i=line.find("\n") #locate the last newline...yes, this requires there is at least one.
          while True:
            j=line.find("\n",i+1)
            if (j==-1): break;
            i=j;
          line = '<group visible="no">'+line[:j]+"</group>"+line[j:] #insert close tag just before it
        elif (namepat.search(lastGroup) is not None):
          #also replace the name in the last (i.e. containing) group
          # with the display text (which we've just spliced out!)
          lastGroup = namepat.sub('name="'+m.group(1)+'"',lastGroup)
        else:
          print "*ERROR* Could not replace group name in",lastGroup
      else:
        print "*ERROR* Could not find label",m.group(1),"from",line
      print lastGroup,
      lastGroup=""
    print line,
