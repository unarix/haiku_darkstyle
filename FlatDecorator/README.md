# Haiku Decorator SDK

## What is this magical place?

This is a repository that shows off how to compile a Haiku decorator without
utilizing the entirety of the Haiku source code. It even includes an experimental Mac OS 8/9 decorator
written by some smart Haiku developers! (Developers are found in the source code headers by the way.)

## How do I use this crazy thing?

1. Clone/download this repository.
2. Open Terminal to the root of this repository.
3. Type "make" and the decorator should start compiling!
4. Your built decorator should appear in a new objects* folder.
5. To test the decorator, copy the built decorator to "/boot/home/config/non-packaged/add-ons/decorators".
6. Open up the Appearance preflet, click on the "Look and Feel" tab, and select
   your decorator from the "Decorator" menu.
7. Cross your fingers that app_server doesn't crash and loads your decorator! :D
