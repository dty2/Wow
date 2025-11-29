#!/bin/sh

DIR="./fixtures/config/"
cd "$DIR" || exit 1

mv config.toml config.toml.backup_
mv config.toml.backup config.toml
mv config.toml.backup_ config.toml.backup
