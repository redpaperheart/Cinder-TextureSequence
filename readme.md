Cinder - Texture Sequence.
===========

A helpful class to controle an image sequence.


Work in progress /  Todo:
--------
 * To only load image files
 * Ask for the saving destination
 * Test loading time and memory usage, shows the outcome in app
 * possibility to switch between timeline and "on update calls"
 * clean up code
 * add some more comments
 * QA


Original code by makemachine http://labs.makemachine.net/
Forked from https://github.com/makemachine/embers/tree/master/TextureSequence

Changed class to use list of *Texture to improve performance when multiple instances of the same png-sequence is in use.