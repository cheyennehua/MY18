#!/bin/bash

echo "Installing packages..."

apt-get update

apt-get install -y \
  build-essential \
  gcc-arm-none-eabi \
  git \
  make \
  python3 \

if [ $1 = "clean" ]; then
  rm -rf /var/lib/apt
fi