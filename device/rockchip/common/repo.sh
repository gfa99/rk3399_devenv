#!/bin/bash

#../repo/repo sync -c
#.repo/repo/repo sync -c

REPO=.repo/repo/repo

$REPO sync -c

while [ $? -ne 0 ] ; 
do  
	$REPO sync -c
done
