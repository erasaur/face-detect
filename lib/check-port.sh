#!/bin/bash

sudo lsof -iTCP:5555 -sTCP:LISTEN
