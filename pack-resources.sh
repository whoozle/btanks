#!/bin/sh
cd data
zip -0 -r ../resources.dat * -x \*.svn\* -x \*.wav
cd ..
