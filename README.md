Puny Search
===============

The weakest search around!

Alright - it's actually a web based interface to a few commands
(mlocate, file, stat, and thunar).

# Screenshot
![screenshot](http://ahungry.com/img/puny-search.png)

# Customize

View the config.h file and adjust the system calls as desired (just
change the binaries if you like - for instance, if you don't want
Thunar to be your default file manager that opens from these searches,
switch it to Nautilus or your system appropriate file manager).

# Install

To compile, simply run 'make' in the repository and start the program
by running:

```
./puny-search 8888
```

Then in your web browser, visit:

http://localhost:8888/

Try out some searches!

# Caveats

This is a very early implementation and really just a light wrapper of
calls to other system utilities, aggregated in the web browser.

Certain things are not implemented yet (parsing of non-alphanumerics
in the search, mlocate's regex style search features).

# Copyright

Matthew Carter <m@ahungry.com>

# License

GPLv3 or later
