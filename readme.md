Cinder - Texture Sequence.
===========

A helpful class to controle an image sequence.


Work in progress /  Todo:
--------
 * use TextureRef instead of pointers
 * possibility to switch between timeline and "on update calls"
 * clean up code
 * add some more comments
 * add block xml file
 * create samples 
 * QA


Original code by makemachine http://labs.makemachine.net/
Forked from https://github.com/makemachine/embers/tree/master/TextureSequence

Changed class to use list of *Texture to improve performance when multiple instances of the same png-sequence is in use.