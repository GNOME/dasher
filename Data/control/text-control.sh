       #!/bin/bash
        for i in $( grep -l " (symbols)" control.*xml); do
            echo item: $i
			sed -e '''
			s/ (symbols)/ (ascii)/g
			s/&#9209;/Stop/g
			s/&#9208;/Pause/g
			s/&#10018;/Move/g
			s/&#8678;/\&lt;=/g
			s/&#8680;/=\&gt;/g
			s/&#128464;/All/g
			s/&#128463;/Page/g
			s/&#182;/Para./g
			s/.&#8265;/Sent./g
			s/&#9473;/Line/g
			s/&#11052;/Word/g
			s/&#8226;/Char/g
			s/&#10683;/Delete/g
			s/&#9003;/\&lt;x/g
			s/&#8998;/x\&gt;/g
			s/&#9114;/Clear/g
			s/&#128203;/Copy/g
			s/&#127381;/New/g
			s/&#128259;/Repeat/g
			s/&#128483;/Speak/g
			''' <$i >$(basename $i .xml).txt.xml 
        done
		sed -e 's_<control name="Default (ascii)">_<control name="">_' <control.generic.txt.xml >control.xml
        rm control.generic.txt.xml
