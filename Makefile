#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := SigilVM

# include $(IDF_PATH)/make/project.mk
# include ./make/project.mk

prepare_build:
	mkdir -p ./build
	cmake CMakeLists.txt -B ./build
