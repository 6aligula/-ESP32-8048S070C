#!/bin/bash

idf.py fullclean
idf.py build
idf.py flash monitor
