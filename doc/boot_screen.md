TODO(Barach)

Imagemagik
```
convert input.png \
  -resize 800x480 \
  -background black \
  -gravity center \
  -extent 800x480 \
  -depth 8 \
  -colors 224 \
  -type truecolor \
  output.tga
```