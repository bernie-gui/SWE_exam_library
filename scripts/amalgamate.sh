#!/bin/bash

amalgamate -w "*.hpp" -i . -i io -i network -i include -v master.hpp amalgamate.hpp
