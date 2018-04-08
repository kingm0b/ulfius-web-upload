# IoT - Upload files with libulfius

A simple modification of 'example_programs/sheep_counter.c' of libulfius.

Tested in *Raspbian Stretch Lite* on Raspberry Pi Model B.

Installing dependencies:

```
# wget https://github.com/babelouest/orcania/releases/download/v1.2.0/liborcania-dev_1.2.0_Raspbian_stretch_armv6l.deb \
https://github.com/babelouest/yder/releases/download/v1.2.0/libyder-dev_1.2.0_Raspbian_stretch_armv6l.deb \
https://github.com/babelouest/ulfius/releases/download/v2.3.1/libulfius-dev_2.3.1_Raspbian_stretch_armv6l.deb

# apt update
# apt install -y libjansson4 libjansson-dev libmicrohttpd12 libmicrohttpd-dev

# dpkg -i *.deb

# ldconfig -v

```

Compiling with:

```
# make
```



