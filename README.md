# Optimal set of 360-degree videos for viewport-adaptive streaming

## Compilation

This software was only tested on linux (archlinux).

Requirements:
* cmake
* a c++ compiler that support *c++11* features
* CPLEX
* Boost
* OpenCV

Tested with:
* gcc 7.1.1
* Boost 1.64
* Opencv 3.2.0

Command lines used to compile the software::

```bash
  mkdir build
  cd build
  cmake .. -DCMAKE_MODULE_PATH=..
  make
```

This will generate an executable named *reprocessing*

## Usage

```bash
  ./reprocessing -c config.ini
```
with *config.ini* a INI configuration file.

A *python3* server and client is available to manage multiple instance of the
optimization software. The server generate multiple parameters for to generate
multiple INI configuration files, and distribute those parameters to clients
script connected with the server.

You can find the synthax to generate manually a configuration file inside the
*RunOptiClient.py* script.

### Use the python3 server and client

You need the *Pyro4* python3 module installed on your machine or inside a
virtualenv. A Pyro4 nameserver need to be started on the host that will run the
server before starting the server.

The variable *pathToPreparedDataset* contains the path to the dataset from
http://dash.ipv6.enstb.fr/headMovements/
