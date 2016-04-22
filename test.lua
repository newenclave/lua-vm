
--print( mysin( 90 ), '\n' )

function test_call(  )
    print( "Hello!\n" )
end

set_callback( 'wptr.calls.test', test_call )

wptr.calls.test( )

print( { 1233, ["1"]=119 } )
