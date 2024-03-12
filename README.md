# ephosweeper
This is minesweeper clone that uses one dimensional array of uint8_t's to hold the data. I got this project to the point that I'm not interested to continue with it anymore. I know that there is some bugs in the gameplay but the idea I wanted to test works just fine.

### Dependencies
[Raylib](https://github.com/raysan5/raylib) is the new sdl!

### Lesson learned
It is cool but useless to represent 2d array in one dimesion. div() uses division (surprise surprise) so it's not so efficient way to calculate x, y coordinates. Allthough it is not a problem with modern computers. Theres plenty of room in 60fps :)
