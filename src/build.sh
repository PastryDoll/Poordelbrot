# #!/bin/sh

# set -xe

# CFLAGS="-Wall -Wextra -std=c++17"
# LIBS="-lglfw -lGL -lm -lGLEW"

# clang++ $CFLAGS -o main main.cpp $LIBS

#!/bin/sh

set -xe

CFLAGS="-O3 -std=c++17"

# Use pkg-config to get the necessary flags for the libraries
LIBS=$(pkg-config --cflags --libs glfw3 glew)

# Compile and link your program
clang++ $CFLAGS -o main main.cpp $LIBS -framework OpenGL