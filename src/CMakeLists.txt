add_dependencies(genmod logger binstream phaser fifoout utils)
add_dependencies(s3mmod genmod)
# add_dependencies(stmmod genmod phaser fifoout utils)
add_dependencies(modmod genmod)
