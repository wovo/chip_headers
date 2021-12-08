#############################################################################
# 
# Makefile that
#    - takes care of renumbering and codelite_workfile
#    - defers other commands to the subdirectories
#
# (c) Wouter van Ooijen (www.voti.nl) 2019-2021
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at 
# http://www.boost.org/LICENSE_1_0.txt)
# 
#############################################################################

ifeq ($(OS),Windows_NT)
   MAKE := bmptk-make
else
   MAKE := make
endif

.PHONY: renumber clean build run error notab codelite_workspace

build:
	$(MAKE) -f makefile.link build-subdirs
   
clean:
	$(MAKE) -f makefile.link clean-subdirs
   
notab:
	$(MAKE) -f makefile.link notab-subdirs
   
renumber:
	$(MAKE) -f makefile.link renumber

codelite_workspace:
	$(MAKE) -f makefile.link codelite_workspace
   
 
# user error handling   

run: error

MSG  = You are trying to run in a library directory.
MSG += Make one of the project source files your current editor file.

error:
	$(error $(MSG) )     
	
		