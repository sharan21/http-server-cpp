#!/bin/bash
g++ -fopenmp http-client.cpp -o client
g++ -fopenmp http-server-poll.cpp -o server -g