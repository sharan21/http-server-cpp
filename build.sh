#!/bin/bash
g++ -fopenmp http-client.cpp -o client
g++ -fopenmp http-server-multi.cpp -o server -g