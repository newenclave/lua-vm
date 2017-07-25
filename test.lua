
--print( mysin( 90 ), '\n' )

function callback( data )
    print( "callback with!", data )
end

local t = get_test( )

print(t.a, t.b, t.c)

print(t.a:mess( ), t.b:mess( ))

print( t.a:get( ):mess( ) )

print(1 >> 100)
