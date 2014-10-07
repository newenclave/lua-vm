
--print( mysin( 90 ), '\n' )

--print( mysin( 'sdfsdf' ), '\n')

function test( a )
    print(a, 'test!\n')
end

local t = {}
t[1] = "x"
t[2] = "y"
t.x = 1
t[30] = 23
t[4] = 45
t['table'] = { }
t['table'].z = 'zzzzzz'
t.z = '!!!!'
t.f = print
t.empty = {}

a = t

print( 1, 2, "123\n", t, "\n===============\n" )
