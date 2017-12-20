cmake_minimum_required(VERSION 3.7)
project(MiniSQL1_1_0)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        API.cpp
        API.h
        BPlusTree.h
        BufferManager.cpp
        BufferManager.h
        BufferStream.h
        CatalogManager.cpp
        CatalogManager.h
        DataStruct.h
        IndexManager.cpp
        IndexManager.h
        Interpreter.cpp
        Interpreter.h
        main.cpp
        RecordManager.cpp
        RecordManager.h
        Uncopyable.h)

add_executable(MiniSQL1_1_0 ${SOURCE_FILES})