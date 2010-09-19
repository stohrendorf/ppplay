add_dependencies(logger utils)
add_dependencies(genmod logger binstream phaser fifoout utils)
add_dependencies(s3mmod genmod)
