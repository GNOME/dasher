# Dasher 5 for macOS

The macOS version of Dasher has been updated (April 2020 and November 2023-September 2024) to build in 64-bit, run on modern versions of macOS, and to improve Direct Mode. These changes were sponsored by Jorge Tendeiro (http://socraticlife.com.au).

## Using Dasher for macOS

**WARNING: Dasher for macOS is beta quality software. This means it may not always do what it is supposed to do. In fact, it could mess up files you choose to edit with it.**

**You should play around a bit with Dasher on inconsequential files to learn the best way to use it; remember to save often and make backup files.**

### Launching Dasher

If you get the error 'Dasher.app can't be opened because it is from an unidentified developer' or '“Dasher” can’t be opened because Apple cannot check it for malicious software.', locate the Dasher app in Finder, ctrl-click, and choose Open. This only needs to be done the first time you run Dasher.

### Basic operation of Dasher

An explanation of how to use the Dasher panel is [here](http://www.inference.org.uk/dasher/Novice.html).

### Direct Mode

Direct Mode can be toggled on and off on the Dasher menu, or by holding down Dasher’s icon in the dock and selecting Direct Mode.

If Direct Mode is off, text is written into the edit box at the top of the Dasher window.

If Direct Mode is on, select the target application to send text to using the dropdown at the top of the Dasher window, or click on the target window to give it input focus.

**For Direct Mode to work, you must grant both Dasher and the target application permission to use Accessibility features in macOS.** You may be prompted to do this when you first enable Direct Mode. If you need to do it later, visit `System Settings` → `Privacy and Security` → `Accessibility`, under "Allow the applications below to control your computer", and add/enable Dasher and the target applications you want to use. You may wish to add all your applications.

If you have a previous version of Dasher in that list, the permission may not apply to a new version of Dasher saved in a different location. You need to remove the existing Dasher entry using the `-` button, then add the new Dasher application.

When Direct Mode is on, clicking on the Dasher window will not show its menu bar. This is so you can easily interact with the target application while using Dasher. You can reveal Dasher's menus by clicking on its icon in the dock, or by selecting 'Dasher' in the drop-down list of target applications in the main Dasher window.

Dasher will remain 'on top' of other applications when in Direct Mode.

If you experience problems with certain characters failing to 'type' correctly into the target application, try setting your keyboard layout to US keyboard in System Preferences. This should allow Dasher to generate the correct characters in Direct Mode for all alphabets.

### Other features and controls

Please explore Dasher → Settings/Preferences to see the many other settings that can be changed.

Of particular note are:

*   Language pane - to choose the language you write in. (If any characters fail to appear in the target app in Direct Mode, ensure you have a US English keyboard selected in System Settings.)
*   Control pane - options for how to control zooming in Dasher
*   Appearance pane - to change Dasher's character display sizes and color scheme.
*   Application pane - options for speech generation, and to enable Control Mode. Control mode introduces a special box that appears at the end of the alphabet, that allows certain editing and control operations to be done from within the Dasher interface

## Training texts

Dasher requires a large amount of sample 'training text' to make efficient predictions. Standard training texts are included for some, but not all, of the alphabets/languages available. Training texts for other alphabets/languages are available on the Dasher website at [https://inference.org.uk/dasher/Download.html](https://inference.org.uk/dasher/Download.html).  To install a training text, place the text file in your `~/Library/Application Support/Dasher` folder. The name of the file corresponds to the alphabet you select in the Language pane of the Settings.

Dasher will learn your own writing style, by adding everything you write in Dasher to a training text file in `~/Library/Application Support/Dasher`. You can tune Dasher to your own writing style and vocabulary by adding a large sample of your own writing to the file that Dasher creates in this folder for the alphabet you are using.

## Further information

Please visit our [website](https://www.inference.org.uk/dasher/) for more information on Dasher.

To report bugs or make a contribution to Dasher development, please visit [https://gitlab.gnome.org/GNOME/dasher](https://gitlab.gnome.org/GNOME/dasher).

Yours,

The Dasher Team