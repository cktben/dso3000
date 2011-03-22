env = Environment()
env.ParseConfig('pkg-config --cflags --libs libusb')
env.ParseConfig('pkg-config --cflags --libs libusb-1.0')
env.ParseConfig('pkg-config --cflags --libs libpng')

env.Program('gpib_console', ['gpib_console.c'])
env.Program('cgi-screenshot', ['cgi-screenshot.cpp', 'dso3000.cpp'])
