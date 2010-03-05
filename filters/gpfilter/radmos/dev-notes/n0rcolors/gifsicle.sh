#!/bin/sh

n0rgif="nexrad_mosaic.gif";
n0rcolors="n0r.colors.gif"
output="result.gif"

gifsicle $n0rgif $n0rcolors | gifsicle -U "#1" > $output
