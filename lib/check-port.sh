#!/bin/bash

for i in {5555..5557}
do
  sudo lsof -iTCP:$i -sTCP:LISTEN
done
