CODEROOT = /afs/cats.ucsc.edu/users/s/rbrounle/cmps181/project1/p2/codebase

#CC = gcc
CC = g++

#CPPFLAGS = -Wall -I$(CODEROOT) -g     # with debugging info
CPPFLAGS = -Wall -I$(CODEROOT) -g -std=c++0x  # with debugging info and the C++11 feature
