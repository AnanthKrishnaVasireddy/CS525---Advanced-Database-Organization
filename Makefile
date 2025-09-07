CC = gcc
CFLAGS = -Wall -g

TARGET1 = test_assign1
TARGET2 = test_assign1_extra

SRC1 = test_assign1_1.c dberror.c storage_mgr.c
SRC2 = test_assign1_extra.c dberror.c storage_mgr.c

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2)

clean:
	rm -f $(TARGET1) $(TARGET2) *.o test_pagefile*.bin
