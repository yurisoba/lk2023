cmake_minimum_required(VERSION 3.26)
project(lk2023)

add_compile_options(-Wall -Werror -Wpedantic)
add_executable(stree a-stree/main.c)
add_executable(rbtest a-stree/rbtest.c a-stree/rbtree.c)
