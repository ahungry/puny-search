# Puny Search - The weakest search around!
# Copyright (C) 2016 Matthew Carter <m@ahungry.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC=gcc
CFLAGS=-c -Wall -std=gnu99

all: puny-search

puny-search: puny-search.o
	$(CC) puny-search.o -o puny-search

puny-search.o: puny-search.c
	$(CC) $(CFLAGS) puny-search.c

clean:
	rm *o puny-search
