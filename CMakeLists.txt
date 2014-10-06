cmake_minimum_required( VERSION 2.8 )

set( PROJECT_NAME lua_test )

project( ${PROJECT_NAME} )

list( APPEND src . )

foreach( dir ${src} )
    aux_source_directory( ${dir} src_dirs )
endforeach(dir)

add_executable( ${PROJECT_NAME} ${src_dirs} )

find_package( Lua51 )

if( ${LUA51_FOUND} )

include_directories( ${LUA_INCLUDE_DIR} )
target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )

else( )

    find_package( Lua50 )

    if( ${LUA50_FOUND} )
        include_directories( ${LUA_INCLUDE_DIR} )
        target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES} )
    endif( )

endif( )

message( ${LUA_INCLUDE_DIR} )
