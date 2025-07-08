# Haiku-OS FlatMode

## Sources:
The binaries here, are only available for 64bits, but you can download the code and compile it using "make" command in the root of the path (decorator and controlLook). Mabe for 32bits need to make changes in the source code (no tested in 32bits!).

### Install of controlLook
Copy the binary "FlatControlLook" (https://github.com/unarix/haiku_darkstyle/tree/master/FlatControlLook/objects.x86_64-cc8-release) in the path: "/boot/home/config/non-packaged/add-ons/control_look" (if not exist the folder, create it).

### Install of Decorator
Copy the binary "FlatDecorator" (https://github.com/unarix/haiku_darkstyle/tree/master/FlatDecorator/objects.x86_64-cc8-release) in the path: "/boot/home/config/non-packaged/add-ons/decorators" (if not exist the folder, create it).

### Install Themes
To use the theme you need to install the package "Theme Manager" from haiku-depot. After install it, copy the folder "DarkFlat"/"LightFlat"/"MoonFlat" (Example: https://github.com/unarix/haiku_darkstyle/tree/master/DarkFlat) in the directory /boot/home/config/settings/UIThemes. Open Theme manager and select one of them from the list.

### Select Decorator and ControlLook from Appearance
In the Appearance preferences dialog, select "Flat" from the combos "Decorator" and "ControlLook"

### Restart
You need to restart the computer or quit "Deskbar" and "Tracker" services from "ProcessControler" (they will restart auto).

![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/Backup/appearence.png?raw=true)
![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/Backup/checkboxes.png?raw=true)
![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/Backup/defbutton.png?raw=true)

## Themes
Are three flat themes to use with the decorators and controllok:

### DarkFlat
![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/DarkFlat/screenshot.png?raw=true)

### LightFlat
![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/LightFLat/screenshot.png?raw=true)

### MoonFlat
![alt text](https://raw.githubusercontent.com/unarix/haiku_darkstyle/master/MoonFLat/screenshot.png?raw=true)

# Guide
How to make ControlLooks & Decorators in Haiku:
http://www.unarix.com.ar/lab/guide/haiku/decorator/controllok/darktheme/2020/06/20/Haiku-Decorators.html
