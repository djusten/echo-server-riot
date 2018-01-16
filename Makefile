# Created by Diogo Justen (diogojusten@gmail.com) in January 2018
# Copyright (C) 2018 Diogo Justen. All rights reserved.

DIR = src
VERSION=1.0

all:
	for dir in $(DIR); do $(MAKE) RIOT_DIR=${RIOT_DIR} -C $$dir $@ || exit $$?; done

clean:
	for dir in $(DIR); do $(MAKE) RIOT_DIR=${RIOT_DIR} -C $$dir $@; done

#EOF
