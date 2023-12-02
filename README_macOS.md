# Dasher 5 for macOS

The macOS build of Dasher has been updated (April 2020 and November 2023) to build in 64-bit and run on modern versions of macOS. These changes were sponsored by Jorge Tendeiro (http://socraticlife.com.au).

## Using Dasher for macOS

**WARNING: Dasher for macOS is beta quality software. This means it may not always do what it is supposed to do. In fact, it could mess up files you choose to edit with it.**

**You should play around a bit with Dasher on inconsequential files to learn the best way to use it; remember to save often and make backup files.**

### Basic operation of Dasher

An explanation of how to use the Dasher panel is [here](http://www.inference.org.uk/dasher/Novice.html).

### Direct Mode

Direct Mode can be toggled on and off on the Dasher menu.

If Direct Mode is off, text is written into the edit box at the top of the Dasher window.

If Direct Mode is on, select the target application to send text to using the dropdown at the top of the Dasher window.

For Direct Mode to work, you must grant both Dasher and the target application permission to use Accessibility features in macOS. You should be prompted to do this when you first enable Direct Mode. If you need to do it later, visit System Preferences → Privacy and Security → Accessibility, under "Allow the applications below to control your computer", and add/enable Dasher and the target applications you want to use.

If you have a previous version of Dasher in that list, the permission may not apply to a new version of Dasher saved in a different location. You need to remove the existing Dasher entry using the `-` button, then add the new Dasher application.

### Other features and controls

Please explore Dasher → Settings/Preferences to see the many other settings that can be changed.

Of particular note are:

*   Language pane - to choose the language you write in
*   Control pane - options for how to control zooming in Dasher
*   Application pane - options for speech generation, and to enable Control Mode. Control mode introduces a special box that appears at the end of the alphabet, that allows certain editing and control operations to be done from within the Dasher interface

## Training texts

Dasher requires a large amount of sample 'training text' to make efficient predictions. Standard training texts are included for some, but not all, of the alphabets/languages available. Training texts for other alphabets/languages are available on the Dasher website at http://inference.org.uk/dasher/Download.html .  To install a training text, place the text file in your `~/Library/Application Support/Dasher` folder. The name of the file corresponds to the alphabet you select in the Language pane of the Settings.

Dasher will learn your own writing style, by adding everything you write in Dasher to a training text file in `~/Library/Application Support/Dasher`. You can tune Dasher to your own writing style and vocabulary by adding a large sample of your own writing to the file that Dasher creates in this folder for the alphabet you are using.

## Further information

Please visit our [website](http://www.inference.phy.cam.ac.uk/dasher/) for more information on Dasher.

To report bugs or make a contribution to Dasher development, please visit Dasher project github page at https://github.com/dasher-project/dasher or https://gitlab.gnome.org/GNOME/dasher


Yours,

The Dasher Team