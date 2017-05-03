#!/bin/bash

dirName="322_submission_`whoami`" ;
origDir=`pwd` ;

# Copy all files to an empty directory
mkdir ../$dirName ;
cd ../$dirName ;
cp -r $origDir/* ./ ;

# Remove unnecessary files
rm -r lib Makefile scripts bin ;
rm -r */tests ;
rm -r */obj ;
rm -r */bin ;
rm -r */scripts ;
rm  */Makefile ;

# Create the package
cd ../ ;
tar cfj ${dirName}.tar.bz2 ${dirName} ;
mv ${dirName}.tar.bz2 src ;
rm -r ${dirName} ;
